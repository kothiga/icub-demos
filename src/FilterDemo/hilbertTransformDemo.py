import numpy as np
from scipy.signal import hilbert
from scipy import fftpack

import matplotlib.pyplot as plt

import argparse

# Init a parser.
parser = argparse.ArgumentParser(description='filter')
parser.add_argument('--rate', default=48000, type=int,  help='Sampling rate                  (default: {})'.format(48000))
parser.add_argument('--freq', default=500,   type=int,  help='Frequency                      (default: {})'.format(500))
parser.add_argument('--samp', default=4096,  type=int,  help='Number of Samples              (default: {})'.format(4096))

def genPureTone(freq, samp, rate):
    t = np.linspace(0, samp*2.0*np.pi/rate, samp)
    pureTone = np.sin(freq * t)
    return pureTone


args = parser.parse_args()

rate = args.rate
freq = args.freq
samp = args.samp

x = genPureTone(freq, samp, rate)
x = x + genPureTone(20, samp, rate)

plt.figure()
plt.plot(x)


y0 = hilbert(x)
Xf = fftpack.fft(x, x.shape[-1], axis=-1)

print(Xf[1], np.iscomplex(Xf))


#y1 = np.abs(y0)

#print("y0 complex? {}".format(np.iscomplexobj(y0)))
#print("y1 complex? {}".format(np.iscomplexobj(y1)))
#
#
#
#plt.figure()
#plt.plot(y0)
#plt.plot(y1)
#
#plt.show()