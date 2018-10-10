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
import random
try:
    f_tmp = open('winsize_filelist.txt', 'rb')
    files = csv.reader(f_tmp)
except Exception:
    print 'winsize_filelist.txt open failed'
    exit()

xdata = []
ydata = []
for f in f_tmp:
    # print i
    xdata01 = [[], []]
    ydata01 = [[], []]
    f = f.replace('\n', '')
    resfile = f.replace('winsize.txt', 'results.csv')
    print resfile
    try:
        f1 = open(resfile, 'rb')
        f1r = csv.reader(f1)
    except Exception:
        print f1, 'open failed'
    for item in f1r:
        break
    for item in f1r:
        is_drop = int(item[0])
        # print is_drop
        ydata01[is_drop].append(is_drop)
        xdata01[is_drop].append(item[1:])
    if f1:
        f1.close()
    del f1r
    gc.collect()
    # break

    minlength = min(len(ydata01[0]), len(ydata01[1]))
    random.shuffle(xdata01[0])
    random.shuffle(xdata01[1])
    random.shuffle(ydata01[0])
    random.shuffle(ydata01[1])
    xdata += xdata01[0][:minlength] + xdata01[1][:minlength]
    ydata += ydata01[0][:minlength] + ydata01[1][:minlength]
    del xdata01, ydata01
    gc.collect()
    # break
try:
    f2 = open('lastresults.csv', 'wb')
    f2w = csv.writer(f2)
except Exception:
    print f2, 'open failed'
    exit()
lengthtmp = len(xdata)
for i in range(0, lengthtmp):
    tmp = [ydata[i]] + xdata[i]
    # tmp = [ydata[i], xdata[i]]
    f2w.writerow(tmp)
if f2:
    f2.close()
parameters = ["drop_count", "busy_time", "ext_busy_time",
       "rx_time", "tx_time", "scan_time", "freq", "noise",
       "bytes", "packets", "qlen", "backlog",
       "drops", "requeues", "overlimits"]

clf = tree.DecisionTreeClassifier(
    criterion='entropy', min_samples_split=200,
    min_samples_leaf=4000)  # 信息熵作为划分的标准，CART
(x_train, x_test, y_train, y_test) = train_test_split(
    xdata, ydata, test_size=0.3)
# print y_train
clf = clf.fit(x_train, y_train)
# print "2"
dot_data = StringIO()
# print "3"

tree.export_graphviz(
    clf, out_file=dot_data,
    feature_names=parameters,
    class_names=['0', '1'],
    filled=True, rounded=True, special_characters=True)
# print "4"
graph = pydotplus.graph_from_dot_data(dot_data.getvalue())
# print "here"
# 导出
# graph.write_pdf('sport.pdf')
graph.write_png('sport.png')
graph.write('abc')
strtree = graph.to_string()
# print strtree
strtree = re.split("\n", strtree)
begin = 0
nodes = []
para = set()
# print len(strtree), "aaa"
for i in strtree:
    # print i
    lief = True
    if i.find("&le") > 0:
        # print "here", i
        lief = False
    i = re.split(" ", i)
    try:
        x = int(i[0])
    except Exception:
        # print i
        continue
    # print x, "xxxxx"
    try:
        (x, y, z) = (i[0], i[1], i[2])
        # print x, y, z
        x = int(x)
        # print y, "yy"
        try:
            z = z.replace(";", "")
            z = int(z)
        except Exception:
            pass
        if y == "->":
            if (len(nodes[len(nodes) - 1]) == 4):
                nodes[len(nodes) - 1] = (x, ) + \
                    nodes[len(nodes) - 1]
        else:
            if lief is False:
                try:
                    tmp = str(i)
                    index1 = tmp.find("=<") + 2
                    index2 = tmp.find("<br/>")
                    tmp1 = tmp[index1:index2]
                    tmp1 = tmp1.replace("'", "")
                    tmp1 = tmp1.replace(",", " ")
                    tmp1 = tmp1.replace("  ", " ")
                    tmp1 = re.split(" ", tmp1)
                    index5 = tmp.find("class")
                    index5 = tmp.find("=", index5) + 5
                    index6 = tmp.find(">]")
                    tmp3 = tmp[index5:index6]
                    tmp3 = tmp3.replace(" ", "")
                    (left, right) = (
                        str(tmp1[0]), float(tmp1[2]))

                    if int(x) == 0 and begin == 0:
                        nodes.append(
                            (0, x, left, right, int(tmp3)))
                        begin = 1
                    else:
                        nodes.append(
                            (x, left, right, int(tmp3)))
                    para.add(left)
                except Exception:
                    # exit()
                    pass
            else:
                try:
                    tmp = str(i)
                    index5 = tmp.find("class")
                    index5 = tmp.find("=", index5) + 5
                    index6 = tmp.find(">]")
                    tmp3 = tmp[index5:index6]
                    tmp3 = tmp3.replace(" ", "")
                    nodes.append((x, "leaf", 0, int(tmp3)))
                except Exception:
                    print "error2"
                    # exit()
    except Exception:
        pass
# print nodes
# print links
# stringtosend = str(nodes)
# print "wwwwwwww", nodes
stringtosend = ""
for i in nodes:
    strtmp = ""
    for k in i:
        if strtmp == "":
            strtmp = str(k)
        else:
            strtmp = strtmp + str(",") + str(k)
    if stringtosend == "":
        stringtosend = strtmp
    else:
        stringtosend = stringtosend + str("|") + strtmp

stringtosend = 'AAAA8B' + stringtosend + ';.'
print stringtosend
# self.decisiontreedict[mac] = stringtosend
print len(clf.feature_importances_), len(parameters)
# exit()
for i in range(0, len(parameters)):
    print parameters[i], ':', clf.feature_importances_[i]
# print clf.feature_importances_
# print len(clf.feature_importances_)
answer = clf.predict(x_train)
precision, recall, fscore, support = score(y_train, clf.predict(x_train))
print('precision: {}'.format(precision))
print('recall: {}'.format(recall))
print('fscore: {}'.format(fscore))
print('support: {}'.format(support))

del xdata, ydata
gc.collect()

if f_tmp:
    f_tmp.close()
