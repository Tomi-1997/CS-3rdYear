EQ_SPEED = 1700
ACC = 1.622

def getAcc(speed):
    n = abs(speed) / EQ_SPEED;
    ans = (1 - n) * ACC;
    return ans;

def abs(x):
    return x if x > 0 else -x
