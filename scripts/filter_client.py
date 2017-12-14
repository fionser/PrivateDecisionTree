import sys
import numpy as np
def mean_std(time):
    m = np.mean(time)
    sd = np.std(time)
    print(m, sd)

n = int(sys.argv[1])
ENC = []
DEC = []
ALL = []
for i in range(n):
    sys.stdin.readline()
    sys.stdin.readline()
    sys.stdin.readline()
    times = sys.stdin.readline().split(' ')
    ENC.append(float(times[0]))
    DEC.append(float(times[1]))
    ALL.append(float(times[2]))
    sys.stdin.readline()
mean_std(ENC)
mean_std(DEC)
mean_std(ALL)
