import sys
import numpy as np
def mean_std(time):
    m = np.mean(time)
    sd = np.std(time)
    print(m, sd)

n = int(sys.argv[1])
EVAL = []
ALL = []
for i in range(n):
    sys.stdin.readline()
    times = sys.stdin.readline().split(' ')
    EVAL.append(float(times[0]))
    ALL.append(float(times[1]))
    sys.stdin.readline()
mean_std(EVAL)
mean_std(ALL)
