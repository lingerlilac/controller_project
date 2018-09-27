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
    fil_winsize = codecs.open("720winsize.txt", "r", 'utf_8_sig')
    # fil6 = codecs.open("channel_ssid_time.csv", "w", 'utf_8_sig')
    winsize = csv.reader(fil_winsize)
    # write_ssid = csv.writer(fil6)
except Exception:
    print "winsize_filelist open failed"
    exit()
ratio = 1000
for f in winsize:
    f = f[0]
    f = f.replace('\n', '')
    f = f.replace(' ', '')
    print f
    rtt_down_record_rtt = []
    rtt_down_record_sequence = []
    rtt_down = {}
    rtt_up = {}
    try:
        f_tmp = open(f, 'rb')
        f_w = csv.reader(f_tmp)
        # x = f.replace('winsize', 'retranstimes')
        # rtt_f = open(x, 'wb')
        # x_w = csv.writer(rtt_f)
    except Exception:
        print f_tmp, 'open failed'
    for i in f_w:
        # print i
        break
    sequence_dic = {}
    # print "abc"
    time_list = []
    for i in f_w:
        (mac_addr, eth_src, eth_dst, ip_src, ip_dst,
            srcport, dstport, sequence, ack_sequence,
         windowsize, cal_windowsize, time,
         datalength, flags, kind, length, wscale) = i
        datalength = int(datalength)
        if datalength == 0:
            continue
        tmp = [int(time), 0]
        time_list.append(tmp)
    if f_tmp:
        f_tmp.close()
    g = f.replace('winsize', 'retranstime')
    try:
        f_x = open(g, 'rb')
        f_r = csv.reader(f_x)
    except Exception:
        print f_x, 'open failed'
    retrans_list = []
    for i in f_r:
        time = i[0]
        tmp = [int(time), 1]
        retrans_list.append(tmp)
    timesandretrans = sorted(retrans_list + time_list)
    # print len(timesandretrans), len(retrans_list), len(time_list)
    min_time = timesandretrans[0][0]
    min_time = min_time - (min_time % ratio)
    length = len(timesandretrans)
    max_time = timesandretrans[length - 1][0]
    max_time = max_time - (max_time % ratio) + ratio
    sizeoflist = int(max_time / ratio) - int(min_time / ratio)
    data_list = [[0, 0, 0] for i in range(0, sizeoflist)]
    for i in timesandretrans:
        (time, sign) = i
        time_origin = time
        time = time - min_time
        tm = int(time / ratio)
        sign = int(sign)
        data_list[tm][0] = time_origin - (time_origin % ratio)
        if sign == 0:
            data_list[tm][1] += 1
        else:
            data_list[tm][2] += 1

    g = f.replace('winsize', 'timesretrans')
    try:
        f_y = open(g, 'wb')
        f_k = csv.writer(f_y)
    except Exception:
        print f_y, 'open failed'
    for i in data_list:
        if i[0] != 0:
            f_k.writerow(i)
    if f_y:
        f_y.close()
    del timesandretrans, retrans_list, time_list
    gc.collect()
    # break
if fil_winsize:
    fil_winsize.close()
