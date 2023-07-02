## To generate somewhat believeable digit images, run this program for around 100+ epochs.
##
## This program saves a generated fake image every epoch to the given folder name below.
## At the end of the training, it generates 100 believeable images and saves it to the disk.

import os
import time

import torch
import torchvision
import matplotlib.pyplot as plt

folder_name = "mnist_gen\\"
def main():
    trainable, _ = get_data()
    ## Mnist image flattened dimension = h x w
    output_dimension = trainable.data.size(1) * trainable.data.size(2)

    ## From a vector of arbitrary size, generate a picture with the same dimension as the mnist data.
    gen = Generator(input_dim = 100, output_dim = output_dimension)

    ## Input is a mnist-like numpy array, output is 0/1
    disc = Discriminator(input_dim = output_dimension)
    train(trainable, gen, disc, epochs = 200, lr = 0.0002, batch_size = 32)


def plot_tensor(x):
    plt.imshow(x.permute(1, 2, 0)) ## [CH, X, X] --> [X, X, CH]
    plt.show()


def get_data():
    mean = 0.5 ## Center around 0.5 as the pixel values are from 0 to 1
    std  = 0.5
    transform = torchvision.transforms.Compose([torchvision.transforms.ToTensor(),
                                               torchvision.transforms.Normalize(mean = mean, std = std)])
    # Train
    train_d = torchvision.datasets.MNIST(root = './data/', train = True, transform = transform, download = True)
    # Test
    test_d = None
    return train_d, test_d


def train(trainable, gen, disc, epochs, lr, batch_size):
    if epochs < 0: return
    if not os.path.exists(folder_name):
        os.makedirs(folder_name)

    train_loader = torch.utils.data.DataLoader(dataset = trainable, batch_size = batch_size, shuffle = True)
    loss_func = torch.nn.BCELoss() # Mean squared error between each element of two inputs
    disc_opt = torch.optim.Adam(disc.parameters(), lr = lr)
    gen_opt  = torch.optim.Adam(gen.parameters(),  lr = lr)
    counter = 1

    for e in range(1, epochs + 1):

        start = time.time()
        gen_loss  = 0
        disc_loss = 0
        size = 0

        for batch_num, (X, y) in enumerate(train_loader):
            gen_loss  += gen_train(gen, disc, X, gen_opt, loss_func, lr, batch_size)
            disc_loss += disc_train(gen, disc, X, disc_opt, loss_func, lr)
            size += 1

        gen_loss = gen_loss / size
        disc_loss = disc_loss / size

        time_diff = time.time() - start
        time_took = f'{time_diff:.2f}s per epoch'
        print(f'[{e} / {epochs}]: Generator loss - {gen_loss:.4f}, Discriminator loss = {disc_loss:.4f}, {time_took}')

        ## Save a random image each epoch to show progression
        fake = gen.generate_digit(1)
        path = folder_name + '\\all\\' + str(counter) + '.png'
        torchvision.utils.save_image(fake.view(fake.size(0), 1, 28, 28), path)
        counter += 1

    ## Save best images after training
    save_many('output', gen, 100, True, disc)


def gen_train(gen, disc, X, opt, loss_func, lr, batch_size):
    gen.zero_grad() ## Reset optimizers'gradient

    fake = gen.generate_digit(batch_size)
    true_vector = torch.ones((batch_size, 1))

    disc_guess = disc(fake)

    ## The loss is actually how much samples fooled the discriminator
    loss = loss_func(disc_guess, true_vector)
    loss.backward()
    opt.step()

    return loss.data.item()


def disc_train(gen, disc, X, disc_opt, loss_func, lr):
    disc.zero_grad()

    real_x = X.view(-1, disc.input_dim)       ## Flatten MNIST images
    batch_size = real_x.shape[0]
    true_vector = torch.ones((batch_size, 1)) ## Actual digit images, so it's filled with 1

    disc_guess = disc(real_x)
    real_loss = loss_func(disc_guess, true_vector) ## Expecting discriminator to guess them to be real

    fake = gen.generate_digit(batch_size)          ## Get generated batch of numbers
    false_vector = torch.zeros_like(true_vector)   ## Fill vector with 0's

    disc_guess = disc(fake)
    fake_loss = loss_func(disc_guess, false_vector) ## Now expecting discriminator to guess them all to be false

    loss = real_loss + fake_loss
    loss.backward()
    disc_opt.step()

    return loss.data.item()


def save_many(epoch, generator, amount, believeable = False, disc = None):
    folder = f'{folder_name}{epoch}'
    if not os.path.exists(folder):
        os.makedirs(folder)

    for i in range(1, amount + 1):
        fake = generator.generate_digit(1) if not believeable else generator.generate_believable(disc)
        path = f'{folder}\\{i}.png'
        torchvision.utils.save_image(fake.view(fake.size(0), 1, 28, 28), path)


class Generator(torch.nn.Module):
    def __init__(self, input_dim, output_dim):
        super(Generator, self).__init__()
        self.input_dim = input_dim
        self.conv1 = torch.nn.Conv1d(input_dim, 382, 3, stride=1, padding=1)
        self.conv2 = torch.nn.Conv1d(382, 784, 3, stride=1, padding=1)


    def forward(self, x, batch_norm = True):
        n = x.shape[0]
        x = x.view(n, self.input_dim, 1)
        x = self.conv1(x)
        x = torch.nn.functional.relu(x)

        x = self.conv2(x)
        x = torch.tanh(x)
        x = x.view(n, 1, 28, 28)
        return x


    def generate_digit(self, batch_size):
        random_noise = torch.autograd.Variable(torch.randn(batch_size, self.input_dim))
        noise_to_image = self.forward(random_noise, batch_norm = False)
        size = noise_to_image.shape[0]
        return noise_to_image.view(size, -1)


    def generate_believable(self, disc):
        """Generates a digit photo which fooled the given discriminator.
        Limited to 50 attempts before returning a digit which may have failed."""
        best_fool = -1
        best_fake = None
        for _ in range(50):
            fake = self.generate_digit(1)
            guess = disc(fake)
            if guess > best_fool:
                best_fool = guess
                best_fake = fake
        return best_fake


class Discriminator(torch.nn.Module):
    def __init__(self, input_dim):
        super(Discriminator, self).__init__()
        self.input_dim = input_dim
        self.fc1 = torch.nn.Linear(input_dim, 256)
        self.fc2 = torch.nn.Linear(256, 1)


    def forward(self, x):
        x = self.fc1(x)
        x = torch.nn.functional.leaky_relu(x, 0.2)
        x = torch.nn.functional.dropout(x, 0.25)
        x = self.fc2(x)
        x = torch.sigmoid(x)
        return x


if __name__ == '__main__':
    main()