import matplotlib.pyplot as plt
import numpy as np

import csv

t = [] # column 0
data1 = [] # column 1
data2 = [] # column 2

with open('sigD.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t.append(float(row[0])) # leftmost column
        data1.append(float(row[0])) # second column
        data2.append(float(row[1])) # third column
h = [
    0.001863628570826028,
    0.002023422419853015,
    0.002356263120875395,
    0.002876754248511092,
    0.003595467073465461,
    0.004518512401304881,
    0.005647217814325138,
    0.006977921053731371,
    0.008501887284797333,
    0.010205354740931122,
    0.012069709832391433,
    0.014071789325939416,
    0.016184303749884953,
    0.018376372851667763,
    0.020614160826358849,
    0.022861596232788318,
    0.025081159099797079,
    0.027234715768298379,
    0.029284380572818788,
    0.031193382582133111,
    0.032926915320149568,
    0.034452947686592114,
    0.035742975186750403,
    0.036772692038445881,
    0.037522566714134509,
    0.037978305943442169,
    0.038131195079572668,
    0.037978305943442169,
    0.037522566714134509,
    0.036772692038445881,
    0.035742975186750410,
    0.034452947686592114,
    0.032926915320149568,
    0.031193382582133115,
    0.029284380572818791,
    0.027234715768298386,
    0.025081159099797089,
    0.022861596232788321,
    0.020614160826358859,
    0.018376372851667763,
    0.016184303749884950,
    0.014071789325939428,
    0.012069709832391433,
    0.010205354740931129,
    0.008501887284797347,
    0.006977921053731373,
    0.005647217814325143,
    0.004518512401304881,
    0.003595467073465460,
    0.002876754248511094,
    0.002356263120875395,
    0.002023422419853015,
    0.001863628570826028,
]

#for i in range(len(data1)):
    # print the data to verify it was read
    #print(", " + str(data1[i]) + ", " + str(data2[i]))


dt = 1.0/10000.0 # 10kHz
#t = np.arange(0.0, 1.0, dt) # 10s
t = data1
# a constant plus 100Hz and 1000Hz
#s = 4.0 * np.sin(2 * np.pi * 100 * t) + 0.25 * np.sin(2 * np.pi * 1000 * t) + 25
s = data2


x = len(h)
data3 = []
for jj in range(x-1):
    data3.append(0)
for i in range(x-1,len(data1)):
    runninglist=[]
    for ii in range(x-1):
        runninglist.append(data2[i-ii]*h[ii])
    
    data3.append(sum(runninglist))

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

fig, (ax1,ax2) = plt.subplots(2, 1)
#ax1.set_title("Signal by time and FFT for sigA")
ax1.set_title('Signal by Time and FFT, filtered and unfiltered('+str(x)+ ' points) of sigD')
ax1.plot(t,y,'k')
ax1.plot(t,y2,'r')
ax1.set_xlabel('Time')
ax1.set_ylabel('Amplitude')
ax2.loglog(frq,abs(Y),'k') # plotting the fft
ax2.loglog(frq2,abs(Y2),'r')
ax2.set_xlabel('Freq (Hz)')
ax2.set_ylabel('|Y(freq)|')


plt.show()

