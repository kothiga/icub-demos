import numpy as np

def HzToErbRate(Hz):
    return (21.4 * np.log10(4.37e-3 * Hz + 1))

def ErbRateToHz(Erb):
    return (10**(Erb / 21.4) - 1) / 4.37e-3

minCF    = 440  #500
maxCF    = 2800 #2800
numChans = 128

minERB = HzToErbRate(minCF)
maxERB = HzToErbRate(maxCF)

cfs = np.linspace(minERB, maxERB, numChans)

print("CFS pre: \n {}".format(cfs))

cfs = ErbRateToHz(cfs)

print("\n\nCFS post: \n {}".format(cfs))





#cfs = ErbRateToHz(linspace(HzToErbRate(mincf),HzToErbRate(maxcf),numchans));
