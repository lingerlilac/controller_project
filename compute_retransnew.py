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
count_i = 0
count_j = 0
file_dic = {}

for i in winsize:
    count_i += 1
    i = i[0]
    i = i.replace('\n', '')
    # print i[0:13]
    key = i[0:15]
    try:
        file_dic[key].append(i)
    except Exception:
        file_dic[key] = [i]

for i in other:
    count_j += 1
    i = i[0]
    i = i.replace('\n', '')
    i_origin = i
    key = i[0:15]
    try:
        file_dic[key].append(i)
    except Exception:
        k1 = i[0:11]
        print k1
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
for winsizef in winsizeandother.keys():
    otherf = winsizeandother[winsizef]
    # xx = '/home/mysql/home/lin/data/bak_new_data/' + winsizef
    # yy = '/home/mysql/home/lin/data/bak_new_data/' + \
    #     winsizeandother[winsizef]
    xx = winsizef
    yy = winsizeandother[winsizef]
    writefile = 'new/' + yy.replace('other.txt', 'retrans.csv')
    print writefile
    try:
        fil1 = open(xx, "r")
        fil3 = open(yy, "r")
        fil2 = open(writefile, 'w')
        write_record = csv.writer(fil2)
    except Exception:
        print "open", xx, yy, "failed"
        exit()
    print xx, yy, writefile
    # continue
    retrans_dic = {}
    retrans_pkt = []
    min_timex = 0
    min_key = ""
    time_winsize = fil1.readline()
    time_other = fil3.readline()
    time_winsize = re.split(",", time_winsize)
    time_other = re.split(",", time_other)
    time_winsize = time_winsize[1]
    time_other = time_other[1]
    time_winsize = int(time_winsize)
    time_other = int(time_other)
    if fil1:
        fil1.close()

    if fil3:
        fil3.close()
    # print time_other, time_winsize
    # exit()
    # file_write = winsizef + "retrans.csv"
    # try:
    #     fil2 = codecs.open(file_write, "w", 'utf_8_sig')
    #     # fil6 = codecs.open("channel_ssid_time.csv", "w", 'utf_8_sig')
    #     write_record = csv.writer(fil2)
    #     # write_ssid = csv.writer(fil6)
    # except Exception:
    #     print "tranningdata open failed"
    #     exit()
    # print time_winsize, time_other, max(time_winsize, time_other)
    time_need = max(time_winsize, time_other)

    min_timex = time_need

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
            key = str(int(sequence)) + "." + str(int(ack_sequence))
            # key = float(key)
            timex = int(timex)
            if timex < time_need:
                continue
            datalength = int(datalength)
            if datalength == 0:
                continue
            key = sequence
            try:
                tmp = retrans_dic[key]
                retrans_dic[key].append(timex)
            except Exception:
                retrans_dic[key] = [timex]
    for key in retrans_dic.keys():
        if len(retrans_dic[key]) > 1:
            retrans_pkt += retrans_dic[key][0:(len(retrans_dic[key]) - 1)]
    # print retrans_pkt
    retrans_pkt = sorted(retrans_pkt)
    # write_record.writerows([retrans_pkt])
    for i in retrans_pkt:
        # print i
        write_record.writerow([i])
    del retrans_dic, retrans_pkt, min_timex,
    del min_key, time_winsize, time_other
    # gc.collect()
    # if fil2:
    #     fil2.close()
    # exit()

gc.collect()
if fil_winsize:
    fil_winsize.close()
if fil_other:
    fil_other.close()
if fil2:
    fil2.close()
