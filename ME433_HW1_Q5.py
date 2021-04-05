import matplotlib.pyplot as plt
import numpy as np

import csv

t = [] # column 0
data1 = [] # column 1
data2 = [] # column 2

with open('sigA.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t.append(float(row[0])) # leftmost column
        data1.append(float(row[0])) # second column
        data2.append(float(row[1])) # third column


#for i in range(len(data1)):
    # print the data to verify it was read
    #print(", " + str(data1[i]) + ", " + str(data2[i]))


dt = 1.0/10000.0 # 10kHz
#t = np.arange(0.0, 1.0, dt) # 10s
t = data1
# a constant plus 100Hz and 1000Hz
#s = 4.0 * np.sin(2 * np.pi * 100 * t) + 0.25 * np.sin(2 * np.pi * 1000 * t) + 25
s = data2


x = 25
data3 = []
for jj in range(x-1):
    data3.append(0)
for i in range(x-1,len(data1)):
    runninglist=[]
    for ii in range(x-1):
        runninglist.append(data2[i-ii])
    
    data3.append(sum(runninglist)/len(runninglist))

s2 = data3






Fs = 400000 # sample rate
Ts = 1.0/Fs; # sampling interval
ts = np.arange(0,t[-1],Ts) # time vector
y = s # the data to make the fft from
y2 = s2
n = len(y) # length of the signal
n2 = len(y2)
k = np.arange(n)
k2 = np.arange(n2)
T = n/Fs
T2 = n2/Fs
frq = k/T # two sides frequency range
frq2 = k2/T2
frq = frq[range(int(n/2))] # one side frequency range
frq2 = frq[range(int(n2/2))]
Y = np.fft.fft(y)/n # fft computing and normalization
Y2 = np.fft.fft(y2)/n2
Y = Y[range(int(n/2))]
Y2 = Y2[range(int(n2/2))]

fig, (ax2) = plt.subplots(1, 1)
#ax1.set_title("Signal by time and FFT for sigA")
#ax1.plot(t,y,'b')
#ax1.set_xlabel('Time')
#ax1.set_ylabel('Amplitude')
ax2.loglog(frq,abs(Y),'k') # plotting the fft
ax2.loglog(frq2,abs(Y2),'r')
ax2.set_xlabel('Freq (Hz)')
ax2.set_ylabel('|Y(freq)|')
ax2.set_title('FFT, filtered and unfiltered('+str(x)+ ' points) of sigA')

plt.show()

