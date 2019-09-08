import numpy as np
import matplotlib.pyplot as plt

sr = 48000.0
md = 0.198
c  = 336.628


def sampDelay(rad):
    return ( (md/2.0) / c  ) * (rad + np.sin(rad)) * sr

def sampDelay2(rad):
    print(md)
    x = (md/2.0) / c
    y = (np.sin(rad))
    return  x * y * sr


halfpi = np.pi / 2
a = np.linspace(-halfpi, halfpi, 181)

b = []
for i in a:
    b.append( sampDelay(i) )

plt.figure()
plt.plot(b)


c = []
for i in a:
    t = sampDelay2(i)
    c.append(  t )

plt.figure()
plt.plot(c)



plt.show()