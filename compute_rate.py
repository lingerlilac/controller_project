#-*—coding:utf8-*-
import numpy as np
import gc
import re
import csv
import codecs
from decimal import *
import os
import statsmodels.api as sm
import numpy as np
import matplotlib as mpl
mpl.use('pdf')
import matplotlib.pyplot as plt
try:
    fil_winsize = codecs.open("queue_filelist.txt", "r", 'utf_8_sig')
    # fil6 = codecs.open("channel_ssid_time.csv", "w", 'utf_8_sig')
    winsize = csv.reader(fil_winsize)
    # write_ssid = csv.writer(fil6)
except Exception:
    print "winsize_filelist open failed"
    exit()
for f in winsize:
    f = f[0]
    f = f.replace('\n', '')
    f = f.replace(' ', '')
    print f
    try:
        f_tmp = open(f, 'rb')
        f_w = csv.reader(f_tmp)
    except Exception:
        print f_tmp, 'open failed'
    for i in f_w:
        # print i
        break
    data_list = []
    for i in f_w:
        (time, qid, byte) = i[1:4]
        qid = int(qid)
        if qid != 3:
            continue
        # print time, qid, byte
        time = int(time)
        byte = int(byte)
        tmp = [time, byte]
        data_list.append(tmp)
    data_list = sorted(data_list)
    rate_list = []
    lendatalist = len(data_list)
    for i in range(0, lendatalist - 1):
        j = i
        while(1):
            j = j + 1
            if j > (lendatalist - 2):
                break
            duration = data_list[j][0] - data_list[i][0]
            byte = data_list[j][1] - data_list[i][1]
            if duration > 100:
                rate_tmp = float(byte) / float(duration)
                tmp = [data_list[j][0], rate_tmp]
                rate_list.append(tmp)
                break

    tmp = np.array(rate_list)
    tmp = tmp.T
    plt.rc('font', family='serif', serif='Times')
    plt.rc('text', usetex=True)
    plt.rc('xtick', labelsize=8)
    plt.rc('ytick', labelsize=8)
    plt.rc('axes', labelsize=8)
    # width as measured in inkscape
    width = 3.487
    height = width / 1.618

    fig, ax = plt.subplots()
    fig.subplots_adjust(left=.15, bottom=.16, right=.99, top=.97)

    x = np.arange(0.0, 3 * np.pi, 0.1)
    plt.plot(tmp[0], tmp[1], 'b.')

    ax.set_ylabel('Throughput (bytes)')
    ax.set_xlabel('Time')
    ax.set_xlim(min(tmp[0]), min(tmp[0]) + 1000 * 3600)

    fig.set_size_inches(width, height)
    pic_name = f.replace('queue.csv', 'tpt.pdf')
    fig.savefig(pic_name)
    if f_tmp:
        f_tmp.close()
    break
if fil_winsize:
    fil_winsize.close()
