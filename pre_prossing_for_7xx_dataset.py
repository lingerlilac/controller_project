#-*—coding:utf8-*-
import numpy as np
import gc
import re
import csv
import codecs
from decimal import *
import os
import statsmodels.api as sm
import matplotlib
import matplotlib.pyplot as plt

try:
    fil_winsize = codecs.open("7xxstats.txt", "r", 'utf_8_sig')
    # fil6 = codecs.open("channel_ssid_time.csv", "w", 'utf_8_sig')
    winsize = csv.reader(fil_winsize)
    # write_ssid = csv.writer(fil6)
except Exception:
    print "winsize_filelist open failed"
    exit()
stat_list = []
for f in winsize:
    f = f[0]
    f = f.replace('\n', '')
    f = f.replace(' ', '')
    print f
    try:
        f_tmp = open(f, 'rb')
        f_r = csv.reader(f_tmp)
    except Exception:
        print f_tmp, "open failed"
    for i in f_r:
        # (retrans, station_amount, busy, recv, tran, byte,
        #  packets, qlen, backlog, drops,
        #  requeues, neibours, drop) = i
        # print(retrans, station_amount, busy, recv, tran, byte,
        #       packets, qlen, backlog, drops, requeues, neibours, drop)
        stat_list.append(i)
        # break
    if f_tmp:
        f_tmp.close()
    # break
try:
    f_s = open('statis.csv', 'wb')
    f_w = csv.writer(f_s)
except Exception:
    print f_s, 'openfailed'
f_w.writerows(stat_list)
if f_s:
    f_s.close()
if fil_winsize:
    fil_winsize.close()
gc.collect()
