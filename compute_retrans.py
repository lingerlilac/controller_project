#-*—coding:utf8-*-
import numpy as np
import gc
import re
import csv
import codecs
import matplotlib
import matplotlib.pyplot as plt
from decimal import *
import sys

# maclist = ("04:a1:51:96:ca:83",
#            "04:a1:51:a3:57:1a",
#            "44:94:fc:82:d9:8e",
#            "04:a1:51:a8:6e:c5",
#            "04:a1:51:96:64:cb",
#            "04:a1:51:a0:65:c0",
#            "04:a1:51:a7:54:f9",
#            "04:a1:51:8e:f1:cb")

file_write = sys.argv[2]
file_read = sys.argv[1]
# print sys.argv[1], sys.argv[2]
# exit()
try:
    fil2 = codecs.open(file_write, "w", 'utf_8_sig')
    # fil6 = codecs.open("channel_ssid_time.csv", "w", 'utf_8_sig')
    write_record = csv.writer(fil2)
    # write_ssid = csv.writer(fil6)
except Exception:
    print "tranningdata open failed"
    exit()

try:
    fil1 = open(file_read, "r")
except Exception:
    print "6666 open failed."
    exit()

# try:
#     fil2 = open("6667.txt", "r")
# except Exception:
#     print "6667 open failed."

lines = fil1.readlines()

retrans_dic = {}
retrans_pkt = []

for item in lines:
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
    key = Decimal(key)
    key = int(key)
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
for i in retrans_pkt:
    write_record.writerow((i, ))
# write_record.writerow(retrans_pkt)
del lines, retrans_dic, retrans_pkt
gc.collect()

if fil1:
    fil1.close()

if fil2:
    fil2.close()
