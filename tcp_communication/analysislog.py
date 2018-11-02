import csv
import numpy as numpy
import gc
import re
import matplotlib
import matplotlib.pyplot as plt
try:
    f1 = open('syslog5', 'rb')
    f1r = csv.reader(f1)
except Exception:
    print f1, 'open failed'
lengthset = set()
x = []
y1 = []
y2 = []
y3 = []
data = []
for item in f1r:
    i = item[0][31:]
    i = i.replace(']', '')
    i = i.replace('linger:', '')
    i = i.replace('\t', ' ')
    i = re.split(' ', i)
    length = len(i)

    if length != 6:
        continue
    # print item
    # print i
    # exit()
    # if length in(8, 10, 11, 13):
    #     # print length, ":", i
    #     continue
    # lengthset.add(length)
    # print i
    (time, port, cwnd, rtt, jitter, mss) = i
    # print i
    # exit()
    lengthset.add(port)
    try:
        time = float(time)
        cwnd = int(cwnd)
        rtt = int(rtt)
        jitter = int(jitter)
    except Exception:
        print i

    # port = int(port)
    # print port
    if port == '1430':
        # lengthset.add(port)
        # if time < 300 or time > 310:
        #     continue
        data.append([time, cwnd, rtt, jitter])
data = sorted(data)
for i in data:
    (time, cwnd, rtt, jitter) = i
    x.append(time)
    y1.append(cwnd)
    y2.append(rtt)
    y3.append(jitter)
    # else:
    #     print port
# plt.plot(x, y1, label='cwnd')
# plt.plot(x, y2, label='rtt')
plt.plot(x, y3, label='jitter')
# plt.gca().set_yscale('log')
plt.legend()
plt.show()
print len(x)
print lengthset

if f1:
    f1.close()
gc.collect()
