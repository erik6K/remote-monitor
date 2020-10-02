import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import scipy.fftpack

# Data for plotting
xR, y = np.loadtxt('data_512_2khz.txt', delimiter=',', unpack=True)
y = y[511:]
xR = xR[511:]

# Number of samplepoints
N = 1024 // 2

# sample spacing
T = 1.0 / (13392.857)
x = np.linspace(0.0, N*T, N)


yf = scipy.fftpack.fft(y)
xf = np.linspace(0.0, 1.0/(2.0*T), N//2)

fig, ax = plt.subplots(2)

ax[0].plot(xR, y, '-')
ax[1].plot(xf[1:200], 2.0/N * np.abs(yf[:N//2])[1:200])

ax[0].set(xlabel='t', ylabel='V')

#fig.savefig("plt.png")
plt.show()