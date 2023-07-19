import edu.princeton.cs.introcs.StdDraw;
import java.awt.*;
import java.util.*;

public class Perceptron
{
    public static void main(String[] args)
    {
        ArrayList<Point_> data = new ArrayList<>();
        data = initDataLinear();
//        data = initDataNonLinear();
        guessValue.put(-1, Color.ORANGE);
        guessValue.put(1, Color.RED);

        double lr = 0.5;  // Scale of weight update each iteration
        double theta = 0; // Threshold - dot product of X and W + bias above this threshold will translate to -1, 1 otherwise
        double[] w = new double[]{0.1, 0, 0};

        initStdDraw();
        initDrawerThread(data, w);

        //
        perceptron(w, data, theta, lr);
        verifyWeights(w, data, theta);
    }

    private static void initDrawerThread(ArrayList<Point_> data, double[] w)
    {
        // Draw data and weights while the algorithm is running
        new Thread(() ->
        {
            while (true)
            {
                drawData(data, w); // Make sure to only read data and w
                delay(fps);
            }
        }).start();
    }


    static HashMap<Integer, Color> guessValue = new HashMap<>();
    static final long fps = 1000 / 144; // Drawing thread refresh rate
    static final int algoDelayMS = 250; // Perceptron delay after weight update

    private static void verifyWeights(double[] w, ArrayList<Point_> data, double theta)
    {
        int c = 0;
        for (Point_ p : data)
        {
            int guess = 1;
            if (w[1] * p.x1 + w[2] * p.x2 + w[0] > theta)
            {
                guess = -1;
            }
            c = guessValue.get(guess) == p.label ? c + 1 : c;
        }
        System.out.println("Correct - " + c + " / " + data.size());
    }

    private static void perceptron(double[] w, ArrayList<Point_> data, double theta, double lr)
    {
        System.out.println("Running perceptron");
        boolean mistaken = true;
        while (mistaken)
        {
            mistaken = false;
            for (Point_ p : data)
            {

                int guess = 1;
                if (w[1] * p.x1 + w[2] * p.x2 + w[0] > theta)
                {
                    guess = -1;
                }

                if (guessValue.get(guess) != p.label)
                {
                    w[0] = w[0] + lr * guess;
                    w[1] = w[1] + lr * guess * p.x1;
                    w[2] = w[2] + lr * guess * p.x2;
                    mistaken = true;
                    delay(algoDelayMS);
                }

            }
        }
    }

    private static void drawData(ArrayList<Point_> data, double[] w)
    {
        StdDraw.clear(Color.black);
        for (Point_ p : data)
        {
            StdDraw.setPenColor(p.label);
            StdDraw.circle(p.x1, p.x2, 0.01);
        }

        StdDraw.setPenColor(Color.GREEN.darker());
        double slope = -(w[0]/w[2])/(w[0]/w[1]);
        double intercept = -w[0]/w[2];
        StdDraw.line(-1, slope * -1 + intercept, 1, slope * 1 + intercept);

        StdDraw.show(0); // No flickering
    }

    private static ArrayList<Point_> initDataLinear()
    {
        ArrayList<Point_> data = new ArrayList<>();
        for (int i = 0; i < 50; i++)
        {
            double x1, x2;
            do
            {
                x1 = Math.random() * 2 - 1; // -1 to 1
                x2 = Math.random() * 2 - 1;
            } while(x1 + x2 > - 0.3);
            data.add(new Point_(x1, x2, Color.RED));
        }

        for (int i = 0; i < 50; i++)
        {
            double x1, x2;
            do
            {
                x1 = Math.random() * 2 - 1; // -1 to 1
                x2 = Math.random() * 2 - 1;
            } while(x1 + x2 < 0.3);
            data.add(new Point_(x1, x2, Color.ORANGE));
        }
        return data;
    }

    private static ArrayList<Point_> initDataNonLinear()
    {
        ArrayList<Point_> data = new ArrayList<>();

        data.add(new Point_(1, 1, Color.RED));
        data.add(new Point_(1, -1, Color.RED));
        data.add(new Point_(-1, 1, Color.RED));
        data.add(new Point_(-1, -1, Color.RED));

        data.add(new Point_(0.1, 0.1, Color.ORANGE));
        data.add(new Point_(0.1, -0.1, Color.ORANGE));
        data.add(new Point_(-0.1, 0.1, Color.ORANGE));
        data.add(new Point_(-0.1, -0.1, Color.ORANGE));

        return data;
    }

    private static void initStdDraw()
    {
        StdDraw.setPenRadius(0.003);
        StdDraw.setScale(-1, 1);
        StdDraw.clear(Color.black);
    }

    private static void delay(long ms)
    {
        //
        try {Thread.sleep(ms);} catch (InterruptedException e) {e.printStackTrace();}
    }
}

class Point_
{
    double x1, x2;
    Color label;

    public Point_(double x1, double x2, Color label)
    {
        this.x1 = x1;
        this.x2 = x2;
        this.label = label;
    }

}
