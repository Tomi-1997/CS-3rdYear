class PID():
    def __init__(self, P, I , D):
        self.P = P
        self.I = I
        self.D = D

        self.prev_vs = 0
        self.ierr = 0

    def calc_error(self, vs, dvs):
        perr = vs - dvs           ## desired speed versus actual speed
        derr = vs - self.prev_vs  ## diff of prev speed to current speed
        self.ierr += perr         ## summation of errors overtime

        self.prev_vs = vs
        return (self.P * perr) + (self.D * derr) + (self.I * self.ierr)
