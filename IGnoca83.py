import math
import re
# import numpy as np
import gc
import csv
csvfile = file('tranningdatanoiw.csv', 'rb')
lines = csv.reader(csvfile)


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
# test

# x = [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1], [0, 1, 0, 0]]
# y = [1, 2, 3, 2, 2]
# IG = info_gain(x, y, 2)
# for k, v in IG:
#     print k, v
# x = [[1, 0, 0, 1], [0, 1, 1, 1], [0, 0, 1, 0]]
# y = [0, 0, 1]
# IG = info_gain(x, y)
# for k, v in IG:
#     print k, v


y = []
# lines = fil.readlines()
lines = list(lines)
for i in range(1, len(lines)):
    if len(lines[i]) < 4:
        continue
    # print lines[i]
    # lines[i] = lines[i].replace("\n", "")
    # lines[i] = lines[i].replace("[", "")
    # lines[i] = lines[i].replace("]", "")
    # lines[i] = re.split(",", lines[i])
    for j in range(1, len(lines[i]) - 1):
        # print lines[i][j]
        lines[i][j] = float(lines[i][j])
    # if lines[i][0] < 10:
    #     lines[i][0] = 0.0
    # elif lines[i][0] < 50 and lines[i][0] >= 10:
    #     lines[i][0] = 1.0
    # else:
    #     lines[i][0] = 2.0
    y.append(lines[i][0])
# for line in lines:
#     print line
#     print line[1]
data = lines
# print y
# data_t = np.matrix(data).T
# x = ("time_x[index]", "in_activetime", "signel", "signal_avg",
#      "noise", "tx_bitrate", "rx_bitrate", "expected_throughput",
#      "rx_bytesr", "rx_packetsr", "tx_bytesr", "tx_packetsr",
#      "tx_retriesr", "tx_failedr", "busy_timer", "receive_timer",
#      "transmit_timer", "bytes_keepq", "packets_keepq", "qlen_keepq",
#      "backlog_keepq", "drops_keepq", "requeues_keepq", "overlimits_keepq")
x = ("rt", "txrate", "active_time", "busy_time",
     "transmit_timer", "receive_timer", "noise",
     "rx_bytes", "rx_packets", "tx_packets",
     "tx_retries", "tx_failed", "bytes_keepq", "packets_keepq", "qlen_keepq",
     "backlog_keepq", "drops_keepq", "requeues_keepq")
# x = ("data_r", "txrate", "active_time", "busy_time",
#      "transmit_time", "receive_time", "noise")
# x = (data_r[i], txrate, active_time, busy_time,
#      transmit_time, receive_time, noise, bytes_keepq,
#      packets_keepq, qlen_keepq, backlog_keepq,
#      drops_keepq, requeues_keepq)
# print y
# print data
IG = info_gain(data, y)
print IG
for k, v in IG:
    try:
        print x[k], v
    except:
        print k, len(x)

if csvfile:
    csvfile.close()
del data, y
gc.collect()
