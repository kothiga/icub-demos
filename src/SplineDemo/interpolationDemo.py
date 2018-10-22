import numpy as np
from scipy import interpolate 
import matplotlib.pyplot as plt

import argparse

# Init a parser.
parser = argparse.ArgumentParser(description='interpolation')
parser.add_argument('--frlag', default=0,     type=int, help='Number of frames to lag the samples by. (default: {})'.format(0))
parser.add_argument('--freq',  default=48000, type=int, help='Sin Frequency to be generated. (default: {})'.format(48000))
args = parser.parse_args()

frlag = args.frlag
freq  = args.freq

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


c          = 343.36
d          = 0.145
nSamples   = 4096
sampleRate = 48000 
#nBeamsPerHemifield = int(np.ceil((d/c) * sampleRate))     # demos way
nBeamsPerHemifield = int( (d / c) * sampleRate) - 1

nBeams = 2 * nBeamsPerHemifield + 1
nMicAngles = nBeams*2-2

print(" c {}\n d {}\n nsamp {}\n sampRate {}\n".format(c, d, nSamples, sampleRate))
print("nBeamsPerHemifield {}".format(nBeamsPerHemifield))
print("nBeams {}\n".format(nBeams))


#######################################

# Number of angle ranges that beamformer should go through them 
# This means the beams moves from zero to 180 by 1 degree difference. 
# so the spatial resolution of the beams are only 1 degree.

# the prepherial angles zero to 10 degrees and 170 to 180 degrees are ignored.
# It means that beams dont sweep these two area. This resolution in space is 
# done to increase the speed of the algorithm
ANGLE_RESOLUTION = 180    
#startAngle_index = int( 10 * (ANGLE_RESOLUTION / 180))
#endAngle_index   = int(170 * (ANGLE_RESOLUTION / 180))

startAngle_index = int(  0 * (ANGLE_RESOLUTION / 180))
endAngle_index   = int(180 * (ANGLE_RESOLUTION / 180))

print("start idx: {}".format(startAngle_index))
print("end   idx: {}".format(endAngle_index))


# SteeringAngle is composed of angles from 10 to 170 degree by resolution of 1 degree
SteeringAngle =  180.0 * np.arange(startAngle_index, endAngle_index) / (ANGLE_RESOLUTION-1)
SteeringAngle = SteeringAngle * np.pi / 180 # SteeringAngle in degree

# making the delay(time) and sample_delay vector for beamforming
delay = d * (-np.cos(SteeringAngle)) / c #to compensate for the delay in the left and right channel
sample_delay = np.round(delay * sampleRate)

print(SteeringAngle)
print("Steering Angle Shape: {}".format(SteeringAngle.shape))

print(sample_delay)
print("Sample Delay Shape: {}".format(sample_delay.shape))


lags = (c/sampleRate) * np.arange(-nBeamsPerHemifield, nBeamsPerHemifield+1) # NOTE: +1 TO INCLUDE LAST ELEMENT
print("Number of Lags:",lags.shape)

exit()

angles = (1/d) * lags

# Find angles out of range for arcsin.
angle_pos_lo = angles <= -1.0
angle_pos_hi = angles >=  1.0

# Set them in range so no points
# are evaluated as NAN.
angles[angle_pos_lo] = -1.0
angles[angle_pos_hi] =  1.0

angles = np.arcsin(angles)


#######################################


radialResolution_degrees = 1
radialResolution_radians = np.pi/180 * radialResolution_degrees
numSpaceAngles = 360 // radialResolution_degrees
spaceAngles = np.linspace(-np.pi, np.pi-radialResolution_radians, numSpaceAngles)

print("number of Space Angles: {}".format(spaceAngles.shape))

micAngles = np.concatenate( (angles, (np.pi + angles[1:-1]))) # NOTE: DOES NOT INCLUDE FIRST & LAST. SAME AS MATLAB CODE
micAngles = np.roll(micAngles, nBeamsPerHemifield) 
micAngles = np.unwrap(micAngles) - (2*np.pi)

print("Number of Mic Angles: {}".format(micAngles.shape))

exit()

#######################################


beamsX, beamsY = pol2cart(angles, 1)
#compass(beamsX, beamsY)
#plt.show()


#######################################


tempBeams = np.zeros((nBeams, nSamples))

egoSpaceMap_front = np.zeros(nBeams)
egoSpaceMap_back  = np.zeros(nBeams-2)
egoSpaceMap_loRes = np.zeros(nMicAngles)
egoSpaceMap_hiRes = np.zeros(numSpaceAngles)


#######################################


t = np.linspace(0, nSamples*2*np.pi/sampleRate, nSamples)
f = freq

left  = np.sin(f * t)
right = left.copy()

# Artificial Lagging of the frequency.
right = np.roll(right, frlag)

prevBeamCount = 0
beamCount = 0

for b in range(-nBeamsPerHemifield, nBeamsPerHemifield+1):

    tempR = np.roll(right, b)
    tempBeam = left + tempR

    egoSpaceMap_front[beamCount] = np.sqrt( np.sum(tempBeam**2) / nSamples )
    prevBeamCount = beamCount
    beamCount += 1


#######################################


plt.figure()

#plt.ylim(top=1.5)
#plt.ylim(bottom=0.0)

plt.plot(angles, egoSpaceMap_front, 'o')
#plt.show()

egoSpaceMap_back  = np.flip(egoSpaceMap_front[1:-1], axis=0)
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

#plt.figure()
#plt.plot(x, y, 'o')

#plt.figure()
#plt.plot(xi, yi, 'o')

egoSpaceMap_hiRes = yi

def lininterp(x, x1, y1, x2, y2):
    return y1 + ((y2 - y1) * (x - x1)) / (x2 - x1)

mAngle = 1
egoSpaceMap_hiRes_Lin = np.zeros(numSpaceAngles)
for sAngle in range(numSpaceAngles):
    
    if spaceAngles[sAngle] > micAngles[mAngle] and mAngle < nMicAngles-1:
        mAngle += 1

    egoSpaceMap_hiRes_Lin[sAngle] = lininterp(
        spaceAngles[sAngle], 
        micAngles[mAngle-1], 
        egoSpaceMap_loRes[mAngle-1], 
        micAngles[mAngle], 
        egoSpaceMap_loRes[mAngle]
    )

#plt.figure()
#plt.plot(spaceAngles, egoSpaceMap_hiRes_Lin, 'o')

lin_spline_error = np.sum( np.abs(egoSpaceMap_hiRes - egoSpaceMap_hiRes_Lin) )
print("\nError between Linear and Spline: {}".format(lin_spline_error))


plt.show()