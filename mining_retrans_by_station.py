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
import os

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
station_2_file_handle = {}
for k in catalog:
    c_i = "/home/mysql/home/lin/data/data/" + \
        k.replace("\n", "") + str('/iw.txt')
    try:
        fil5 = codecs.open(k.replace("\n", "") + ".csv", "w", 'utf_8_sig')
        # fil6 = codecs.open("channel_ssid_time.csv", "w", 'utf_8_sig')
        write_record = csv.writer(fil5)
        # write_ssid = csv.writer(fil6)
    except Exception:
        print "tranningdata open failed"
        exit()
    tmp = "mkdir " + "/home/mysql/home/lin/data/data/" + \
        k.replace("\n", "") + "/split"
    try:
        os.system(tmp)
    except Exception:
        pass
    # exit()
    with open(c_i, 'rb') as f:
        for res in f:
            res = re.split(", ", res)
            if len(res) < 20:
                continue
            station = res[1]
            station = station.replace(" ", "")
            station = station.replace('"', "")
            mac_tmp = res[2]
            mac_tmp = mac_tmp.replace('"', "")
            res[0] = np.uint64(res[0])
            res[1] = res[1].replace('"', "")
            res[2] = res[2].replace('"', "")

            for i in (3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 16, 17, 18, 19):
                res[i] = int(res[i])
            tmp = res[14]
            tmp = tmp.replace('"', "")
            tmp = re.split(" ", tmp)
            try:
                tmp = float(tmp[0])
            except Exception:
                pass
            res[14] = tmp
            # print tmp
            # exit()
            tmp = res[15]
            tmp = tmp.replace('"', "")
            tmp = re.split(" ", tmp)
            try:
                tmp = float(tmp[0])
            except Exception:
                pass
            res[15] = tmp
            file_name = "/home/mysql/home/lin/data/data/" + \
                k.replace("\n", "") + str('/split/') + station + str('_iw.csv')
            try:
                tmp = station_2_file_handle[station]
            except Exception:
                try:
                    f_tmp = open(file_name, "w")
                    f_handle = csv.writer(f_tmp)
                    station_2_file_handle[station] = f_handle
                    f_handle.writerow(("time", "station", "mac_addr", "device",
                                       "inactive_time",
                                       "rx_bytes", "rx_packets",
                                       "tx_bytes", "tx_packets", "tx_retries",
                                       "tx_failed", "signel", "signal_avg",
                                       "noise", "tx_bitrate", "rx_bitrate",
                                       "active_time", "busy_time",
                                       "receive_time", "transmit_time"))
                except Exception:
                    print "file", file_name, "open failed"
                    exit()

            f_handle = station_2_file_handle[station]
            f_handle.writerow(res)

for f in station_2_file_handle.values():
    if f:
        f.close()

del station_2_file_handle, catalog
gc.collect()
