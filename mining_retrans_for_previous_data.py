#-*—coding:utf8-*-
'''
This file is used to find the longest flow out of
every winsize file
'''
import os
import gc
import csv
import numpy as np

try:
    fil1 = open("/home/lin/working/codes/data/c.txt", "r")
except Exception:
    print "data categories open failed."

catalog = fil1.readlines()

for k in catalog:
    # try:
    #     fil5 = codecs.open(k.replace("\n", "") + ".csv", "w", 'utf_8_sig')
    #     # fil6 = codecs.open("channel_ssid_time.csv", "w", 'utf_8_sig')
    #     write_record = csv.writer(fil5)
    #     # write_ssid = csv.writer(fil6)
    # except Exception:
    #     print "tranningdata open failed"
    #     exit()
    print k
    command = "cd /home/mysql/home/lin/data/data/" + \
        k.replace("\n", "") + "/split/ && " + "ls *winsize.csv > tmp.txt"
    # print command
    os.system(command)
    command = "cat tmp.txt"
    file = "/home/mysql/home/lin/data/data/" + \
        k.replace("\n", "") + "/split/" + "tmp.txt"
    try:
        fl1 = open(file, 'rb')
        results = fl1.readlines()
    except Exception:
        print "open", file, "failed"

    # print results
    for res in results:
        res = res.replace("\n", "")
        # print res
        flows = {}
        fl2 = ""
        f_tmp = "/home/mysql/home/lin/data/data/" + \
            k.replace("\n", "") + "/split/" + res
        # print res
        retrans_file = f_tmp.replace("winsize", "retranstime")
        try:
            print f_tmp
            fl2 = open(f_tmp, 'rb')
            fl3 = open(retrans_file, 'wb')
            tmp = csv.reader(fl2)
            fl3_writer = csv.writer(fl3)
            lines = [row for row in tmp]
        except Exception:
            print fl2, "open failed"
        length_of_file = len(lines)
        for ll in range(1, length_of_file):
            if (ll % 100000) == 1:
                print ll, length_of_file
            (mac_addr, eth_src, eth_dst, ip_src, ip_dst,
                srcport, dstport, sequence, ack_sequence,
             windowsize, cal_windowsize, time, datalength,
             flags, kind, length, wscale) = lines[ll]
            datalength = int(datalength)
            sequence = int(sequence)
            srcport = int(srcport)
            dstport = int(dstport)
            time = np.uint64(time)
            if datalength == 0:
                continue
            key = (ip_src, ip_dst, srcport, dstport, sequence)
            try:
                flows[key].append(time)
            except Exception:
                flows[key] = [time]
        for key in flows.keys():
            if len(flows[key]) > 1:
                # print flows[key], f_tmp
                for i in range(0, len(flows[key]) - 1):
                    fl3_writer.writerow((flows[key][i],))
        del flows
        gc.collect()
        if fl2:
            fl2.close()
        if fl3:
            fl3.close()
    if fl1:
        fl1.close()

    # print k

del catalog
gc.collect()
if fil1:
    fil1.close()
