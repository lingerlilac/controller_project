#-*—coding:utf8-*-
import numpy as np
import gc
import re
import csv
import codecs
from decimal import *
try:
    fil_winsize = codecs.open("winsize_filelist.txt", "r", 'utf_8_sig')
    # fil6 = codecs.open("channel_ssid_time.csv", "w", 'utf_8_sig')
    winsize = csv.reader(fil_winsize)
    # write_ssid = csv.writer(fil6)
except Exception:
    print "winsize_filelist open failed"
    exit()

try:
    fil_other = codecs.open("other_filelist.txt", "r", 'utf_8_sig')
    # fil6 = codecs.open("channel_ssid_time.csv", "w", 'utf_8_sig')
    other = csv.reader(fil_other)
    # write_ssid = csv.writer(fil6)
except Exception:
    print "other_filelist open failed"
    exit()


# try:
#     filelist = codecs.open("filelist.txt", "w", 'utf_8_sig')
#     # fil6 = codecs.open("channel_ssid_time.csv", "w", 'utf_8_sig')
#     file_writer = csv.writer(filelist)
#     # write_ssid = csv.writer(fil6)
# except Exception:
#     print "filelist open failed"
#     exit()
file_dic = {}

for i in winsize:
    i = i[0]
    i = i.replace('\n', '')
    # print i[0:13]
    key = i[0:15]
    try:
        file_dic[key].append(i)
    except Exception:
        file_dic[key] = [i]

for i in other:
    i = i[0]
    i = i.replace('\n', '')
    i_origin = i
    key = i[0:15]
    try:
        file_dic[key].append(i)
    except Exception:
        k1 = i[0:11]
        # print k1
        for j in file_dic.keys():
            # print j, k1, j.find(k1)
            if j.find(k1) == 0:
                # print file_dic[j][0], j
                x = file_dic[j][0][11:16]
                y = i[11:16]
                # print x, y
                h0 = int(x[0:2])
                m0 = int(x[3:5])
                h1 = int(y[0:2])
                m1 = int(y[3:5])
                hh = (h0 - h1) * 60
                mm = (m0 - m1)
                duration = hh + mm
                if duration < 10 and duration > 0:
                    # print h0, m0, h1, m1
                    file_dic[j].append(i_origin)

# for key in file_dic.keys():
#     # print file_dic[key], file_dic[key][0][11:16]
#     file_writer.writerow((file_dic[key][0], file_dic[key][1]))

winsizeandother = {}
for key in file_dic.keys():
    winsizeandother[file_dic[key][0]] = file_dic[key][1]
del file_dic
ratio = 1000
for winsizef in winsizeandother.keys():
    # xx = '/home/mysql/home/lin/data/bak_new_data/' + winsizef
    # yy = '/home/mysql/home/lin/data/bak_new_data/' + \
    #     winsizeandother[winsizef]
    xx = winsizef
    zz = 'new/' + winsizeandother[winsizef].replace('other.txt', 'retrans.csv')
    yy = zz.replace('retrans', 'ratio')
    # print yy, zz
    # exit()
    # writefile = 'new/' + yy.replace('other.txt', 'ratio.csv')
    # print writefile
    try:
        # fil1 = open(xx, "r")
        fil2 = open(zz, "r")
        fil3 = open(yy, 'w')
        write_record = csv.writer(fil3)
    except Exception:
        print "open", zz, "failed"
        exit()
    print xx, zz, yy
    retrans_times = fil2.readlines()
    begin_time = int(retrans_times[0]) / ratio
    try:
        end_time = int(retrans_times[len(retrans_times) - 100]) / ratio
    except Exception:
        continue
    duration = end_time - begin_time
    print begin_time, end_time, duration
    # break
    wintimes = [0.0 for a in range(duration + 1)]
    datalength_count = [[0, 0] for a in range(duration + 1)]
    with open(xx, 'rb') as f:
        for item in f:
            try:
                (WINSIZE, timex, mac_addr, eth_src, eth_dst,
                 ip_src, ip_dst, sourceaddr, destination,
                 sequence, ack_sequence, windowsize,
                 cal_windowsize, datalength, flags, kind,
                 length, wscale, x) = re.split(",", item)
            except Exception:
                print item
                break
            datalength = int(datalength) + 40
            timex = int(timex)
            timex = timex / ratio
            timex = timex - begin_time
            if timex < 0:
                continue
            if timex > duration:
                continue
            wintimes[timex] += 1.0
            eth_src = eth_src.replace(' ', '')
            eth_dst = eth_dst.replace(' ', '')
            # print eth_src, datalength, 'abc'
            downlink = (eth_src.find('6c:e8:73:57:a5') >= 0)
            datalength_count[timex][downlink] += datalength
            # print wintimes[timex], datalength_count[timex]
            # datalength_count[timex] += datalength
    # print wintimes
    res_dic = {}
    for i in retrans_times:
        i = int(i)
        i = i / ratio
        i = i - begin_time
        try:
            res_dic[i] += 1.0
        except Exception:
            res_dic[i] = 1.0
    wintimes_tmp = {}
    for i in range(0, duration + 1):
        tmp = 0
        try:
            tmp = res_dic[i]
            wintimes_tmp[i] = round(tmp / wintimes[i], 4)
        except Exception:
            wintimes_tmp[i] = 0.0
    # print wintimes
    for i in range(0, duration + 1):
        tmp = (begin_time + i)
        if wintimes[i] > 0:
            write_record.writerow(
                [tmp, wintimes[i], wintimes_tmp[i],
                 datalength_count[i][0], datalength_count[i][1]])
    if fil2:
        fil2.close()
    if fil3:
        fil3.close()
    # break
    del datalength_count, wintimes, wintimes_tmp
    gc.collect()

gc.collect()
if fil_winsize:
    fil_winsize.close()
if fil_other:
    fil_other.close()
