#-*—coding:utf8-*-
import numpy as np
import gc
import re
import csv
import codecs
import matplotlib
import matplotlib.pyplot as plt
from decimal import *
import time as time_linger
import copy


import pandas as pd
from pandas import Series
import numpy as np
import pydotplus
from sklearn import tree
import csv
from sklearn.externals.six import StringIO
from sklearn.metrics import precision_recall_curve
from sklearn.metrics import classification_report
from sklearn.model_selection import train_test_split
from sklearn.metrics import precision_recall_fscore_support as score
import re
import sys
# maclist = ("04:a1:51:96:ca:83",
#            "04:a1:51:a3:57:1a",
#            "44:94:fc:82:d9:8e",
#            "04:a1:51:a8:6e:c5",
#            "04:a1:51:96:64:cb",
#            "04:a1:51:a0:65:c0",
#            "04:a1:51:a7:54:f9",
#            "04:a1:51:8e:f1:cb")

file_write = sys.argv[3]
file_read_time = sys.argv[2]
file_read_information = sys.argv[1]

try:
    fil2 = codecs.open(file_write, "w", 'utf_8_sig')
    # fil6 = codecs.open("channel_ssid_time.csv", "w", 'utf_8_sig')
    write_record = csv.writer(fil2)
    # write_ssid = csv.writer(fil6)
except Exception:
    print "tranningdata open failed"
    exit()

try:
    fil1 = open(file_read_information, "r")
    fil3 = open(file_read_time, "r")
except Exception:
    print "6666 or retranstime open failed."

lines = fil1.readlines()
times = fil3.readlines()

drop = []
queue = []
survey = []
iw = []
beacon = []
data_list = []


class data_linger:

    def __init__(self, is_drop, timex_drop, timex_survery, timex_queue,
                 drop_count, survey_time, time_busy, time_ext_busy,
                 time_rx, time_tx, time_scan,
                 center_freq, noise, bytes, packets, qlen,
                 backlog, drops, requeues, overlimits):
        self.is_drop = is_drop
        self.timex_drop = timex_drop
        self.drop_count = drop_count
        self.timex_survery = timex_survery
        self.survey_time = survey_time
        self.time_busy = time_busy
        self.time_ext_busy = time_ext_busy
        self.time_rx = time_rx
        self.time_tx = time_tx
        self.time_scan = time_scan
        self.center_freq = center_freq
        self.noise = noise
        self.timex_queue = timex_queue
        self.bytes = bytes
        self.packets = packets
        self.qlen = qlen
        self.backlog = backlog
        self.drops = drops
        self.requeues = requeues
        self.overlimits = overlimits


for item in lines:
    length = len(item) - 1
    tmp = int(item[0])
    item = item[3:length]
    item = re.split(", ", item)
    item[0] = int(item[0])
    length = len(item)
    if tmp == 2:
        for i in range(4, length):
            item[i] = int(item[i])
        drop.append(item)
    elif tmp == 3:
        for i in range(3, length - 1):
            item[i] = int(item[i])
        item[length - 1] = float(item[length - 1])
        iw.append(item)
    elif tmp == 4:
        for i in range(2, length):
            item[i] = int(item[i])
        queue.append(item)
    elif tmp == 5:
        for i in range(2, 6):
            item[i] = int(item[i])
        beacon.append(item)
    elif tmp == 6:
        for i in range(2, length):
            item[i] = int(item[i])
        survey.append(item)
    else:
        print "fuck"
drop = sorted(drop)
queue = sorted(queue)
iw = sorted(iw)
survey = sorted(survey)
beacon = sorted(beacon)
last_low_index = 0.2
last_low_index1 = 0.2


def BinSearch(array, key, low, high):
    global last_low_index
    mid = int((low + high) / 2)
    if key == array[mid][0]:  # 若找到
        return array[mid]
    if low > high:
        # return False
        if last_low_index == 0.2:
            return 'error'
        return array[last_low_index]
    if key < array[mid][0]:
        return BinSearch(array, key, low, mid - 1)  # 递归
    if key > array[mid][0]:
        last_low_index = mid
        return BinSearch(array, key, mid + 1, high)


def BinSearch_ind(array, key, low, high):
    global last_low_index1
    mid = int((low + high) / 2)
    if key == array[mid][0]:  # 若找到
        return mid
    if low > high:
        # return False
        if last_low_index1 == 0.2:
            return 'error'
        return last_low_index1
    if key < array[mid][0]:
        return BinSearch_ind(array, key, low, mid - 1)  # 递归
    if key > array[mid][0]:
        last_low_index1 = mid
        return BinSearch_ind(array, key, mid + 1, high)


wait_to_del = set()
for item in times:
    tm = 0
    try:
        tm = int(item)
    except Exception:
        continue

    if tm < int(survey[0][0]):
        continue
    ind = BinSearch_ind(survey, tm, 0, len(survey) - 1)
    if ind == 'error':
        continue
    (timex_survey, mac_addr, survey_time, time_busy, time_ext_busy,
     time_rx, time_tx, time_scan, center_freq, noise) = survey[ind]
    wait_to_del.add(ind)
    dp = BinSearch(drop, tm, 0, len(drop) - 1)
    if dp == 'error':
        continue
    drop_count = dp[len(dp) - 1]
    timex_drop = dp[0]
    qu = BinSearch(queue, tm, 0, len(queue) - 1)
    if qu == 'error':
        continue
    (timex_queue, mac_addr, queue_id, bytes1, packets,
     qlen, backlog, drops, requeues, overlimits) = qu
    # iw1 = BinSearch(iw, tm, 0, len(iw) - 1)
    # be = BinSearch(beacon, tm, 0, len(beacon) - 1)
    dt = data_linger(1, timex_drop, timex_survey, timex_queue,
                     drop_count, survey_time, time_busy, time_ext_busy,
                     time_rx, time_tx, time_scan, center_freq, noise, bytes1,
                     packets, qlen, backlog, drops, requeues, overlimits)
    # print dt.drop_count, dt.survey_time, dt.time_busy
    data_list.append(dt)
    # exit()

wait_to_del = sorted(wait_to_del, reverse=True)

for i in wait_to_del:
    del survey[i]

for item in survey:
    tm = item[0]
    (timex_survey, mac_addr, survey_time, time_busy, time_ext_busy,
     time_rx, time_tx, time_scan, center_freq, noise) = item
    dp = BinSearch(drop, tm, 0, len(drop) - 1)
    if dp == 'error':
        continue
    drop_count = dp[len(dp) - 1]
    timex_drop = dp[0]
    qu = BinSearch(queue, tm, 0, len(queue) - 1)
    if qu == 'error':
        continue
    (timex_queue, mac_addr, queue_id, bytes1, packets,
     qlen, backlog, drops, requeues, overlimits) = qu
    # iw1 = BinSearch(iw, tm, 0, len(iw) - 1)
    # be = BinSearch(beacon, tm, 0, len(beacon) - 1)
    dt = data_linger(0, timex_drop, timex_survey, timex_queue,
                     drop_count, survey_time, time_busy, time_ext_busy,
                     time_rx, time_tx, time_scan, center_freq, noise, bytes1,
                     packets, qlen, backlog, drops, requeues, overlimits)
    # print dt.drop_count, dt.survey_time, dt.time_busy
    data_list.append(dt)
    # exit()

del drop, queue, survey, iw, wait_to_del, lines, times

length = len(data_list)
processed_data = []

tm_last = time_linger.time()
for i in range(0, length):
    found = False
    # tm_current = time_linger.time()
    # tm_last = tm_current
    for j in range(i, length):
        if data_list[j].timex_survery > data_list[i].timex_survery + 1000:
            # print data_list[j].timex_survery - data_list[i].timex_survery
            found = True
            break
    if found is True:
        previous = data_list[i]
        currentd = data_list[j]
        new_data = copy.copy(currentd)
        tmp1 = float(currentd.drop_count - previous.drop_count)
        tmp2 = float(currentd.timex_drop - previous.timex_drop)
        if(tmp2 == 0):
            new_data.drop_count = -1
        else:
            new_data.drop_count = round(tmp1 / tmp2, 4)

        tmp1 = float(currentd.survey_time - previous.survey_time)
        tmp2 = float(currentd.survey_time - previous.survey_time)
        # print tmp2
        if(tmp2 == 0):
            new_data.timex_survery = -1
            new_data.time_busy = -1
            new_data.time_ext_busy = -1
            new_data.time_rx = -1
            new_data.time_tx = -1
            new_data.time_scan = -1
        else:
            new_data.timex_survery = round(tmp1 / tmp2, 4)

            tmp1 = float(currentd.time_busy - previous.time_busy)
            new_data.time_busy = round(tmp1 / tmp2, 4)

            tmp1 = float(currentd.time_ext_busy - previous.time_ext_busy)
            new_data.time_ext_busy = round(tmp1 / tmp2, 4)

            tmp1 = float(currentd.time_rx - previous.time_rx)
            new_data.time_rx = round(tmp1 / tmp2, 4)

            tmp1 = float(currentd.time_tx - previous.time_tx)
            new_data.time_tx = round(tmp1 / tmp2, 4)

            tmp1 = float(currentd.time_scan - previous.time_scan)
            new_data.time_scan = round(tmp1 / tmp2, 4)

        tmp2 = float(currentd.timex_queue - previous.timex_queue) / 1000.0
        if (tmp2 != 0):
            tmp1 = float(currentd.bytes - previous.bytes)
            new_data.bytes = round(tmp1 / tmp2, 4)

            tmp1 = float(currentd.packets - previous.packets)
            new_data.packets = round(tmp1 / tmp2, 4)

            tmp1 = float(currentd.drops - previous.drops)
            new_data.drops = round(tmp1 / tmp2, 4)

            tmp1 = float(currentd.requeues - previous.requeues)
            new_data.requeues = round(tmp1 / tmp2, 4)

            tmp1 = float(currentd.overlimits - previous.overlimits)
            new_data.overlimits = round(tmp1 / tmp2, 4)
        else:
            new_data.bytes = -1
            new_data.packets = -1
            new_data.drops = -1
            new_data.requeues = -1
            new_data.overlimits = -1

        processed_data.append(new_data)
print time_linger.time() - tm_last
del data_list

xdata = []
ydata = []

write_record.writerow(("drop", "drop_count", "busy_time", "ext_busy_time",
                       "rx_time", "tx_time", "scan_time", "freq", "noise",
                       "bytes", "packets", "qlen", "backlog",
                       "drops", "requeues", "overlimits"))
for i in processed_data:
    write_record.writerow((i.is_drop, i.drop_count,
                           i.time_busy, i.time_ext_busy,
                           i.time_rx, i.time_tx, i.time_scan,
                           i.center_freq, i.noise,
                           i.bytes, i.packets, i.qlen, i.backlog,
                           i.drops, i.requeues, i.overlimits))
    ydata.append(i.is_drop)
    xdata.append([i.drop_count,
                  i.time_busy, i.time_ext_busy,
                  i.time_rx, i.time_tx, i.time_scan,
                  i.center_freq, i.noise,
                  i.bytes, i.packets, i.qlen, i.backlog,
                  i.drops, i.requeues, i.overlimits])

tmp = ["drop_count", "busy_time", "ext_busy_time",
       "rx_time", "tx_time", "scan_time", "freq", "noise",
       "bytes", "packets", "qlen", "backlog",
       "drops", "requeues", "overlimits"]

clf = tree.DecisionTreeClassifier(
    criterion='entropy', min_samples_split=200,
    min_samples_leaf=400)  # 信息熵作为划分的标准，CART
(x_train, x_test, y_train, y_test) = train_test_split(
    xdata, ydata, test_size=0.3)
# print y_train
clf = clf.fit(x_train, y_train)
print "2"
dot_data = StringIO()
print "3"

tree.export_graphviz(
    clf, out_file=dot_data,
    feature_names=tmp,
    class_names=['0', '1'],
    filled=True, rounded=True, special_characters=True)
print "4"
graph = pydotplus.graph_from_dot_data(dot_data.getvalue())
print "here"
# 导出
graph.write_pdf('sport.pdf')
graph.write_png('sport.png')
graph.write('abc')
strtree = graph.to_string()
# print strtree
strtree = re.split("\n", strtree)
nodes = []
links = []
# for i in strtree:
#     # print i
#     lief = True
#     if i.find("&le") > 0:
#         # print "here", i
#         lief = False
#     i = re.split(" ", i)
#     try:
#         x = int(i[0])
#     except Exception:
#         continue
#     # print x
#     try:
#         (x, y, z) = (i[0], i[1], i[2])
#         # print x
#         x = int(x)
#         try:
#             z = z.replace(";", "")
#             z = int(z)
#         except Exception:
#             pass
#         if y == "->":
#             links.append((x, z))
#         else:
#             if lief is False:
#                 tmp = str(i)
#                 index1 = tmp.find("=<") + 2
#                 index2 = tmp.find("<br/>")
#                 tmp = tmp[index1:index2]
#                 tmp = tmp.replace("'", "")
#                 tmp = re.split(",", tmp)
#                 # print tmp
#                 (left, right) = (tmp[0], tmp[2])
#                 # right = float(right)
#                 # print left, right
#                 # links.append((x, z, value))
#                 nodes.append((x, left, right))
#             else:
#                 nodes.append(x, "BBB", -1000)
#     except Exception:
#         pass
# print nodes
# print links
# print len(clf.feature_importances_), len(tmp)
# exit()
for i in range(0, len(tmp)):
    print tmp[i], ':', clf.feature_importances_[i]
# print clf.feature_importances_
# print len(clf.feature_importances_)
answer = clf.predict(x_train)
precision, recall, fscore, support = score(y_train, clf.predict(x_train))
print('precision: {}'.format(precision))
print('recall: {}'.format(recall))
print('fscore: {}'.format(fscore))
print('support: {}'.format(support))

del xdata, ydata, processed_data
gc.collect()
if fil1:
    fil1.close()
if fil2:
    fil2.close()
