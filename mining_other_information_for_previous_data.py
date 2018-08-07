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
    fil1 = open("/home/lin/working/codes/data/d.txt", "r")
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
    previous = "/home/mysql/home/lin/data/data/" + \
        k.replace("\n", "") + "/split/"
    file = previous + "tmp.txt"
    try:
        fl1 = open(file, 'rb')
        results = fl1.readlines()
    except Exception:
        print "open", file, "failed"

    # print results
    for res in results:
        res = res.replace('\n', '')
        res = previous + res.replace('winsize', 'retranstime')
        # print res
        rtimes = []
        iw_list = []
        qu_list = []
        dr_list = []
        be_list = []
        f_iw = res.replace('retranstime', 'iw')
        f_queue = res.replace('retranstime', 'queue')
        f_drop = res.replace('retranstime', 'drop')
        f_beacon = res.replace('retranstime', 'beacon')
        try:
            fl2 = open(res, 'rb')
            fl2_reader = csv.reader(fl2)
            rtimes = [row for row in fl2_reader]
            ff_iw = open(f_iw, 'rb')
            ff_queue = open(f_queue, 'rb')
            ff_drop = open(f_drop, 'rb')
            ff_beacon = open(f_beacon, 'rb')
            iw_list = csv.reader(ff_iw)
            qu_list = csv.reader(ff_queue)
            dr_list = csv.reader(ff_drop)
            be_list = csv.reader(ff_beacon)
        except Exception:
            print fl2, "open faied"
            exit()

        del rtimes, iw_list, qu_list, dr_list, be_list
        gc.collect()
        if fl2:
            fl2.close()
        if fl1:
            fl1.close()
        if ff_iw:
            ff_iw.close()
        if ff_queue:
            ff_queue.close()
        if ff_drop:
            ff_drop.close()
        if ff_beacon:
            ff_beacon.close()

    # print k

del catalog
gc.collect()
if fil1:
    fil1.close()
