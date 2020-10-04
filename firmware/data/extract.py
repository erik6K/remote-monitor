import numpy as np

x, y = np.loadtxt('data_512_2khz.txt', delimiter=',', unpack=True)
y = y[511:]

print(y.tolist())