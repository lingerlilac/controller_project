#-*—coding:utf8-*-
'''
This file is used to find the longest flow out of
every winsize file
'''
import numpy as np
import gc
import re
import csv
import codecs

channel_ssid_time = {}
channel_record = {}
calculate = {}

# maclist = ("04:a1:51:96:ca:83",
#            "04:a1:51:a3:57:1a",
#            "44:94:fc:82:d9:8e",
#            "04:a1:51:a8:6e:c5",
#            "04:a1:51:96:64:cb",
#            "04:a1:51:a0:65:c0",
#            "04:a1:51:a7:54:f9",
#            "04:a1:51:8e:f1:cb")


try:
    fil1 = open("/home/lin/working/codes/data/a.txt", "r")
except Exception:
    print "data categories open failed."

catalog = fil1.readlines()
for k in catalog:
    receord_winsize = {}
    record_w = {}
    find_the_max = 0
    find_the_max_index = ""
    c_i = "/home/mysql/home/lin/data/data/" + \
        k.replace("\n", "") + str('/winsize.txt')
    try:
        fil5 = codecs.open(k.replace("\n", "") + ".csv", "w", 'utf_8_sig')
        # fil6 = codecs.open("channel_ssid_time.csv", "w", 'utf_8_sig')
        write_record = csv.writer(fil5)
        # write_ssid = csv.writer(fil6)
    except Exception:
        print "tranningdata open failed"
        exit()
    with open(c_i, 'rb') as f:
        for r in f:
            try:
                (mac_addr, eth_src, eth_dst, ip_src, ip_dst, srcport,
                    dstport, sequence, ack_sequence, windowsize,
                    cal_windowsize, time, datalength,
                    flags, kind, length, wscale) = re.split(",", r)
            except Exception:
                print r
                continue
            mac_addr = mac_addr.replace(" ", "")
            mac_addr = mac_addr.replace('"', "")
            eth_src = eth_src.replace(" ", "")
            eth_src = eth_src.replace('"', "")
            eth_dst = eth_dst.replace(" ", "")
            eth_dst = eth_dst.replace('"', "")
            srcport = int(srcport)
            dstport = int(dstport)
            # cal_windowsize = int(cal_windowsize)
            sequence = int(sequence)
            ack_sequence = int(sequence)
            length = int(length)
            time = np.uint64(time)
            # if length == 0:
            #     continue
            key = (mac_addr, eth_src, eth_dst, srcport,
                   dstport, sequence, ack_sequence)
            try:
                receord_winsize[key].append(time)
            except Exception:
                receord_winsize[key] = [time]

    del f
    for i in receord_winsize.keys():
        if len(receord_winsize[i]) > 1:
            # write_record.writerow(
                # (i, receord_winsize[i][0: len(receord_winsize[i]) - 1]))
            (mac_addr, eth_src, eth_dst, srcport,
             dstport, sequence, ack_sequence) = i
            key = (mac_addr, eth_src, eth_dst, srcport, dstport)
            time = receord_winsize[i][0: len(receord_winsize[i]) - 1]
            try:
                record_w[key] += time
            except Exception:
                record_w[key] = time
    for i in record_w.keys():
        tmp = record_w[i]
        s = set()
        for m in tmp:
            s.add(m)
        record_w[i] = s
    for i in record_w.keys():
        # write_record.writerow((i, len(record_w[i]), record_w[i]))
        if len(record_w[i]) > find_the_max:
            find_the_max = len(record_w[i])
            find_the_max_index = i
    write_record.writerow(
        (k.replace("\n", ""), find_the_max_index[0], find_the_max_index[1],
            find_the_max_index[2], find_the_max_index[3],
            find_the_max_index[4], find_the_max))
    for i in record_w[find_the_max_index]:
        # print i
        write_record.writerow((np.uint64(i), ))

    del record_w, receord_winsize, find_the_max, find_the_max_index
    if fil5:
        fil5.close()
gc.collect()
