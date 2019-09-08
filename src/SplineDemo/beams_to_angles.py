import numpy as np
from scipy import interpolate 
import matplotlib.pyplot as plt

import argparse

# Init a parser.
parser = argparse.ArgumentParser(description='interpolation')
parser.add_argument('--frlag', default=0,     type=int, help='Number of frames to lag the samples by. (default: {})'.format(0))
parser.add_argument('--freq',  default=580, type=int, help='Sin Frequency to be generated. (default: {})'.format(48000))
args = parser.parse_args()

frlag = args.frlag
freq  = args.freq

def lininterp(x, x1, y1, x2, y2):
    return y1 + ((y2 - y1) * (x - x1)) / (x2 - x1)

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

def genBeamformedPureTone(f, l, beamsPerHemi, samples, rate):
    
    beams = 2 * beamsPerHemi + 1
    egoSpaceMap_front = np.zeros(beams)

    t = np.linspace(0, samples*2*np.pi/rate, samples)
    
    left  = np.sin(f * t)
    right = left.copy()

    # Artificial Lagging of the frequency.
    right = np.roll(right, l)

    beamCount = 0

    for b in range(-beamsPerHemi, beamsPerHemi+1):

        #print("Lag={}".format(b))
        tempR = np.roll(right, b)
        tempBeam = left + tempR

        egoSpaceMap_front[beamCount] = np.sqrt( np.sum(tempBeam**2) / samples )
        beamCount += 1

    return egoSpaceMap_front

def genFrontFieldBeamAngles_0(c, d, rate, beamsPerHami):

    angles = (1 / d) * (c / rate) * np.arange(-beamsPerHami, beamsPerHami+1)
    angles[angles <= -1.0] = -1.0
    angles[angles >=  1.0] =  1.0
    angles = np.arcsin(angles)

    return angles

def mirrorFrontField(frontField):

    length = len(frontField)
    #np.zeros(length * 2 - 2)

    print(length, length * 2 - 2)



###########################################
# Main.
###########################################

c          = 336.628
d          = 0.198 #0.19 #0.145
nSamples   = 4096
sampleRate = 48000 
nBeamsPerHemifield = int(np.ceil((d/c) * sampleRate))     # demos way
#nBeamsPerHemifield = int( (d / c) * sampleRate) - 1
angle_resolution = 0.25

nBeams = 2 * nBeamsPerHemifield + 1

BeamformedRmsAudio   = genBeamformedPureTone(freq, frlag, nBeamsPerHemifield, nSamples, sampleRate)
frontFieldBeamAngles = genFrontFieldBeamAngles_0(c, d, sampleRate, nBeamsPerHemifield)

print("Angles the beams are pointed to: {}\n{}\n\n".format(frontFieldBeamAngles.shape, frontFieldBeamAngles))


###########################################
# Display Beam Angles on a Compass.
###########################################
beamsX, beamsY = pol2cart(frontFieldBeamAngles, 1)
compass(beamsX, beamsY)

degFrontFieldBeamsAngles = np.degrees(frontFieldBeamAngles)
print(degFrontFieldBeamsAngles)

plt.figure()
plt.plot(np.linspace(-21, 21, nBeams), degFrontFieldBeamsAngles, 'o')
#plt.ylim(-(nBeamsPerHemifield+2), nBeamsPerHemifield+2)
plt.xticks(
    np.linspace(-nBeamsPerHemifield, nBeamsPerHemifield, 7).tolist()
)
plt.yticks(
    np.linspace(-90, 90, 7).tolist()
)


print(nBeams)


plt.show()

exit()

###########################################
# Project these beam angles onto real angles.
###########################################
numAnglePosition = int(180 * angle_resolution + 1)
frontFieldRealAngles = np.linspace((-np.pi / 2.0), (np.pi / 2.0), numAnglePosition)
print("Real angles of the front field: {}\n{}\n\n".format(frontFieldRealAngles.shape, frontFieldRealAngles))

#backFieldRealAngles = np.flip(frontFieldRealAngles[1:-1], axis=0)
#fullFieldRealAngles = np.concatenate((frontFieldRealAngles, backFieldRealAngles))
#print("Real angles of full field: {}\n{}\n\n".format(fullFieldRealAngles.shape, fullFieldRealAngles))


###########################################
# Interpolate the front field beamformed 
# data to get real angle distribution.
###########################################

frontFieldEgoMap = np.zeros(numAnglePosition)



beamAngle = 0
for realAngle in range(numAnglePosition):

    if frontFieldRealAngles[realAngle] > frontFieldBeamAngles[beamAngle+1]:
        beamAngle += 1

    frontFieldEgoMap[realAngle] = lininterp (
        frontFieldRealAngles[realAngle],   # x
        frontFieldBeamAngles[beamAngle],   # x0
        BeamformedRmsAudio[beamAngle],     # y0
        frontFieldBeamAngles[beamAngle+1], # x1
        BeamformedRmsAudio[beamAngle+1],   # y1
    )

plt.figure()
plt.plot(frontFieldRealAngles, frontFieldEgoMap, 'o')
    
mirrorFrontField(frontFieldEgoMap)

exit()
































plt.show()
