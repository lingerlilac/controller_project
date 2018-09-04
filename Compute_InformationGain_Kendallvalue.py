import math
import re
import gc
import csv
from scipy import stats
import numpy as np
csvfile = file('datalist.csv', 'rb')
lines = csv.reader(csvfile)
# lines = list(lines)
try:
    fil1 = open('pre11.csv', 'wb')
    write1 = csv.writer(fil1)
except:
    print "open record file failed"
    exit()


def info_gain(x, y, k=None):
    num_d = len(y)
    num_ck = {}
    num_fi_ck = {}
    num_nfi_ck = {}
    for xi, yi in zip(x, y):
        num_ck[yi] = num_ck.get(yi, 0) + 1
        for index, xii in enumerate(xi):
            if not num_fi_ck.has_key(index):
                num_fi_ck[index] = {}
                num_nfi_ck[index] = {}
            if not num_fi_ck[index].has_key(yi):
                num_fi_ck[index][yi] = 0
                num_nfi_ck[index][yi] = 0
            if not xii == 0:
                num_fi_ck[index][yi] = num_fi_ck[index].get(yi) + 1
            else:
                num_nfi_ck[index][yi] = num_nfi_ck[index].get(yi) + 1
    num_fi = {}
    for fi, dic in num_fi_ck.items():
        num_fi[fi] = sum(dic.values())
    num_nfi = dict([(fi, num_d - num) for fi, num in num_fi.items()])
    HD = 0
    for ck, num in num_ck.items():
        p = float(num) / num_d
        HD = HD - p * math.log(p, 2)
    IG = {}
    for fi in num_fi_ck.keys():
        POS = 0
        for yi, num in num_fi_ck[fi].items():
            p = (float(num) + 0.0001) / (num_fi[fi] + 0.0001 * len(dic))
            POS = POS - p * math.log(p, 2)

        NEG = 0
        for yi, num in num_nfi_ck[fi].items():
            p = (float(num) + 0.0001) / (num_nfi[fi] + 0.0001 * len(dic))
            NEG = NEG - p * math.log(p, 2)
        p = float(num_fi[fi]) / num_d
        IG[fi] = round(HD - p * POS - (1 - p) * NEG, 4)
    IG = sorted(IG.items(), key=lambda d: d[1], reverse=True)
    if k == None:
        return IG
    else:
        return IG[0:k]


def red(x, IG):
    feature = dict.fromkeys([fi for fi, v in IG])
    newx = []
    for xi in x:
        newrow = []
        for index, xii in enumerate(xi):
            if feature.has_key(index):
                newrow.append(xii)
        newx.append(newrow)
    return newx

y = []
data = []
# lines = fil.readlines()
# lines = list(lines)
for k in lines:
    # if len(k) < 10:
    #     continue
    (ratio, Stations, Channel_utility,
        Receive_utility, Transmit_utility, Bytes,
     Packets, Qlen, Backlog, Drops_in_queue,
     Requeues, Neighbors, Drops_in_mac) = k
    x = [ratio, Stations, Channel_utility,
         Receive_utility, Transmit_utility, Bytes,
         Packets, Qlen, Backlog, Drops_in_queue,
         Requeues, Neighbors, Drops_in_mac]
    for i in range(0, len(x)):
        x[i] = float(x[i])
    data.append(x)
    # retrans = int(retrans)
    y.append(ratio)

x = ("ratio", "Stations", "Channel utility",
     "Receive utility", "Transmit utility", "Bytes",
     "Packets", "Qlen", "Backlog", "Drops in queue",
     "Requeues", "Neighbors", "Drops in mac")
write1.writerow(x)
write1.writerows(data)
IG = info_gain(data, y)
# print IG
print "iGGGGGGGGGGGGGGGGG"
for k, v in IG:
    try:
        print x[k], v
    except Exception:
        print k, len(x)
data = np.matrix(data).T
for i in range(0, len(x)):
    try:
        print x[i], 'a', stats.kendalltau(data[i, ], data[0, ])
    except Exception:
        print i, 'abc'
if csvfile:
    csvfile.close()
if fil1:
    fil1.close()
# del data, y
gc.collect()
