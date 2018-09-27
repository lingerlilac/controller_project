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
    fil_winsize = codecs.open("rttlist.txt", "r", 'utf_8_sig')
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
        rtt_value = int(i[0])
        data_list.append(rtt_value)
    lendatalist = len(data_list)
    jitter_record = []
    for i in range(0, lendatalist - 1):
        j = i + 1
        j_tmp = data_list[j] - data_list[i]
        if j_tmp < 0:
            j_tmp = -j_tmp
        jitter_record.append(j_tmp)
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
    plt.plot(jitter_record, 'b.')

    ax.set_ylabel('Jitters (ms)')
    ax.set_xlabel('Time')
    ax.set_xlim(0, 10000)

    fig.set_size_inches(width, height)
    pic_name = f.replace('rtt.csv', 'jitter.pdf')
    fig.savefig(pic_name)
    # fig.close()
    if f_tmp:
        f_tmp.close()
    break
if fil_winsize:
    fil_winsize.close()
