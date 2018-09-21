import numpy as np
from scipy import interpolate 
import matplotlib.pyplot as plt

def pol2cart(phi, rho):
    x = rho * np.cos(phi)
    y = rho * np.sin(phi)
    return x, y

def compass(u, v, arrowprops=None):
    """
    Compass draws a graph that displays the vectors with
    components `u` and `v` as arrows from the origin.
    """

    def cart2pol(x, y):
        """
        Convert from Cartesian to polar coordinates.
        """
        radius = np.hypot(x, y)
        theta = np.arctan2(y, x)
        return theta, radius

    angles, radii = cart2pol(u, v)
    fig, ax = plt.subplots(subplot_kw=dict(polar=True))
    kw = dict(arrowstyle="->", color='k')
    if arrowprops: kw.update(arrowprops)
    [ax.annotate("", xy=(angle, radius), xytext=(0, 0), arrowprops=kw) for angle, radius in zip(angles, radii)]
    ax.set_ylim(0, np.max(radii))

    return fig, ax




icub = True
icub = False

if icub:
    c          = 338
    d          = 0.145
    nSamples   = 48000
    sampleRate = 48000 
    nBeamsPerHemifield = int(np.floor((d/c) * sampleRate) - 1 ) # icub way
else:
    c          = 336.628
    d          = 0.14
    nSamples   = 48000
    sampleRate = 48000 
    nBeamsPerHemifield = int(np.ceil((d/c) * sampleRate))     # demos way

nBeams = 2 * nBeamsPerHemifield + 1

print(" c {}\n d {}\n nsamp {}\n sampRate {}\n".format(c, d, nSamples, sampleRate))
print("nBeamsPerHemifield {}".format(nBeamsPerHemifield))
print("nBeams {}\n".format(nBeams))


#######################################


lags = (c/sampleRate) * np.arange(-nBeamsPerHemifield, nBeamsPerHemifield+1) # NOTE: +1 TO INCLUDE LAST ELEMENT
print(lags.shape)

angles = (1/d) * lags

# Find angles out of range for arcsin.
angle_pos_lo = angles <= -1.0
angle_pos_hi = angles >=  1.0

# Set them in range so no points
# are evaluated as NAN.
angles[angle_pos_lo] = -1.0
angles[angle_pos_hi] =  1.0

angles = np.arcsin(angles)

print("First Three and Last Three Angles: {} -- {}".format(angles[0:3], angles[-3:]))
print("\n\n", angles)


#######################################


radialResolution_degrees = 1
radialResolution_radians = np.pi/180 * radialResolution_degrees
numSpaceAngles = 360 // radialResolution_degrees
spaceAngles = np.linspace(-np.pi, np.pi-radialResolution_radians, numSpaceAngles)

micAngles = np.concatenate( (angles, (np.pi + angles[1:-1]))) # NOTE: DOES NOT INCLUDE FIRST & LAST. SAME AS MATLAB CODE
micAngles = np.roll(micAngles, nBeamsPerHemifield)
micAngles = np.unwrap(micAngles) - (2*np.pi)

print("\n\nMic angles\n", micAngles, "\n")
print("Mic Angles Shape {}".format(micAngles.shape), "\n\n")


#######################################


beamsX, beamsY = pol2cart(angles, 1)
compass(beamsX, beamsY)
#plt.show()


#######################################


tempBeams = np.zeros((nBeams, nSamples))

egoSpaceMap_front = np.zeros(nBeams)
egoSpaceMap_back  = np.zeros(nBeams-2)
egoSpaceMap_loRes = np.zeros(nBeams*2-2)
egoSpaceMap_hiRes = np.zeros(numSpaceAngles)


#######################################


t = np.linspace(0, nSamples*2*np.pi/sampleRate, nSamples)
f = 3000

left  = np.sin(f * t)
right = left.copy()

prevBeamCount = 0
beamCount = 0

for b in range(-nBeamsPerHemifield, nBeamsPerHemifield+1):

    tempR = np.roll(right, b)
    tempBeam = left + tempR

    egoSpaceMap_front[beamCount] = np.sqrt( np.sum(tempBeam**2) / nSamples )
    prevBeamCount = beamCount
    beamCount += 1


#######################################


#plt.figure()
#plt.plot(egoSpaceMap_front)
#plt.show()

egoSpaceMap_back = np.flip( egoSpaceMap_front[1:-1], axis=0 ) # NOTE: DOES NOT INCLUDE FIRST & LAST. SAME AS MATLAB CODE
egoSpaceMap_loRes = np.concatenate((egoSpaceMap_front, egoSpaceMap_back))

#plt.figure()
#plt.plot(egoSpaceMap_loRes)
#plt.show()

egoSpaceMap_loRes = np.roll(egoSpaceMap_loRes, nBeamsPerHemifield)

#plt.figure()
#plt.plot(egoSpaceMap_loRes)
#plt.show()


#######################################

x    = micAngles
y    = egoSpaceMap_loRes
tck  = interpolate.splrep(x, y, s=0)
xi   = spaceAngles
yi   = interpolate.splev(xi, tck, der=0)

plt.figure()
plt.plot(x, y, 'o')

plt.figure()
plt.plot(xi, yi, 'o')

plt.show()