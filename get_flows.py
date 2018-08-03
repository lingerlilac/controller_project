#-*—coding:utf8-*-
'''
This file is used to get iw, queu, beacon, 
drop information for the flows generated
by mining_retrans.py.
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

flows = []
try:
    fil1 = open("/home/lin/working/codes/data/b.txt", "r")
except Exception:
    print "data categories open failed."
    exit()


def parse_iw(filename, filename_w, mac_addr, eth_src, eth_dst):
    """
    @brief      This function get iw for specified flow out of raw
    iw informations

    @param      filename    raw iw file handle
    @param      filename_w  iw information for specified data flow
    @param      mac_addr    AP address
    @param      eth_src     used to get station address
    @param      eth_dst     used to get station address

    @return     { description_of_the_return_value }
    """
    try:
        f_iw = open(filename, "r")
        f_w = open(filename_w, "w")
        f_write = csv.writer(f_w)
    except Exception:
        print filename, " open failed"
        exit()
    result = f_iw.readlines()
    # print mac_addr, eth_src, eth_dst, filename, filename_w
    # exit()
    for res in result:
        res = re.split(", ", res)
        if len(res) < 20:
            print res
            continue
        # print res
        mac_tmp = res[2]
        mac_tmp = mac_tmp.replace('"', "")
        # print mac_tmp, mac_addr
        if mac_tmp != mac_addr:
            continue
        station = res[1]
        station = station.replace('"', "")
        # print station, eth_src, eth_dst
        if (station != eth_src) and (station != eth_dst):
            continue
        # print mac_addr, station
        # print res
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
        # print res
        # exit()
        f_write.writerow(res)
    if f_iw:
        f_iw.close()
    if f_w:
        f_w.close()


def parse_queue(filename, c_w, mac_tmp):
    """
    @brief      get queue information for specified flow

    @param      filename  queue information handl
    @param      c_w       output file handle
    @param      mac_tmp   AP address

    @return     null
    """
    try:
        f_queue = open(filename, "r")
        f_w = open(c_w, "w")
        f_write = csv.writer(f_w)
    except Exception:
        print filename, " open failed"
        exit()
    result = f_queue.readlines()
    for res in result:
        res = re.split(", ", res)
        try:
            (mac_addr, time, queue_id, bytes, packets,
             qlen, backlog, drops, requeues) = res
        except Exception:
            print res
            continue
        mac_addr = mac_addr.replace('"', "")
        if mac_addr == mac_tmp:
            time = np.uint64(time)
            # print len(res)
            for i in range(2, 9):
                try:
                    res[i] = int(res[i])
                except Exception:
                    pass
            f_write.writerow(res)
    if f_queue:
        f_queue.close()
    if f_w:
        f_w.close()


def parse_beacon(filename, c_w, mac_tmp):
    try:
        f_beacon = open(filename, "r")
        f_w = open(c_w, "w")
        f_write = csv.writer(f_w)
    except Exception:
        print filename, " open failed"
        exit()
    result = f_beacon.readlines()
    for res in result:
        # print res
        res = re.split(", ", res)
        try:
            (time, timestamp, data_rate, current_channel, channel_type,
             ssl_signal, bw, bssid, essid, mac_addr) = res
        except Exception:
            continue
        time = np.uint64(time)
        mac_addr = mac_addr.replace('\n', "")
        mac_addr = mac_addr.replace('"', "")
        if mac_addr != mac_tmp:
            continue
        res[9] = mac_addr
        for i in (1, 2, 3, 5, 6):
            res[i] = int(res[i])
        for i in (4, 7, 8, 9):
            res[i] = res[i].replace('"', "")
        # print res
        f_write.writerow(res)


def parse_dropped(filename, c_w, mac_tmp):
    try:
        f_dropped = open(filename, "r")
        f_w = open(c_w, "w")
        f_write = csv.writer(f_w)
    except Exception:
        print filename, " open failed"
        exit()
    result = f_dropped.readlines()
    for res in result:
        res = re.split(", ", res)
        try:
            (ip_src, ip_dst, mac_addr, port_src, port_dst,
             drop_location, sequence, ack_sequence, time) = res
        except Exception:
            continue
        for i in range(0, 3):
            res[i] = res[i].replace('"', "")
        if res[2] != mac_tmp:
            continue
        for i in range(3, 9):
            res[i] = int(res[i])
        # print res
        # print mac_addr
        f_write.writerow(res)


catalog = fil1.readlines()
for k in catalog:
    """
    Open every file begin with 314, 315, etc. and end with
    .csv, read the first line to get the flow, and then
    get iw, queue, beacon, drop information from raw information
    files.
    """
    receord_winsize = {}
    record_w = {}
    find_the_max = 0
    find_the_max_index = ""

    try:
        fil5 = open(k.replace("\n", "") + ".csv", "r")
        write_record = csv.reader(fil5)
    except Exception:
        print "tranningdata open failed"
        exit()
    for m in write_record:
        break
    tmp = m[0]
    tmp_len = len(tmp)
    tmp = tmp[tmp_len - 3:tmp_len]
    m[0] = tmp
    print m
    (name, mac_addr, eth_src, eth_dst, srcport,
     dstport, tmz) = m
    print name, mac_addr, eth_src, eth_dst, srcport, dstport, tmz
    tmp_string = '/' + mac_addr + "_" + "eth_src" + "_" + \
        "eth_dst" + "_" + str(srcport) + "_" + str(dstport) + "_" + "iw.csv"
    flows.append(m)
    if fil5:
        fil5.close()
    c_i = "/home/mysql/home/lin/data/data/" + \
        k.replace("\n", "") + str('/iw.txt')
    c_w = "/home/mysql/home/lin/data/data/" + \
        k.replace("\n", "") + tmp_string
    parse_iw(c_i, c_w, mac_addr, eth_src, eth_dst)
    c_i = "/home/mysql/home/lin/data/data/" + \
        k.replace("\n", "") + str('/queue.txt')
    tmp_string = '/' + mac_addr + "_" + "eth_src" + "_" + \
        "eth_dst" + "_" + str(srcport) + "_" + str(dstport) + "_" + "queue.csv"
    c_w = "/home/mysql/home/lin/data/data/" + \
        k.replace("\n", "") + tmp_string
    parse_queue(c_i, c_w, mac_addr)
    # exit()
    c_i = "/home/mysql/home/lin/data/data/" + \
        k.replace("\n", "") + str('/beacon.txt')
    tmp_string = '/' + mac_addr + "_" + "eth_src" + "_" + \
        "eth_dst" + "_" + str(srcport) + "_" + \
        str(dstport) + "_" + "beacon.csv"
    c_w = "/home/mysql/home/lin/data/data/" + \
        k.replace("\n", "") + tmp_string
    parse_beacon(c_i, c_w, mac_addr)
    # exit()
    c_i = "/home/mysql/home/lin/data/data/" + \
        k.replace("\n", "") + str('/dropped.txt')
    tmp_string = '/' + mac_addr + "_" + "eth_src" + "_" + \
        "eth_dst" + "_" + str(srcport) + "_" + \
        str(dstport) + "_" + "dropped.csv"
    c_w = "/home/mysql/home/lin/data/data/" + \
        k.replace("\n", "") + tmp_string
    parse_dropped(c_i, c_w, mac_addr)

print flows
if fil1:
    fil1.close()
del flows, catalog
gc.collect()
