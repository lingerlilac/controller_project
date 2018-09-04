#-*—coding:utf8-*-
import numpy as np
import gc
import re
import csv
import codecs
from decimal import *
import os
import matplotlib.pyplot as plt
try:
    fil_winsize = codecs.open("/home/oneT/data/list.txt", "r", 'utf_8_sig')
    # fil6 = codecs.open("channel_ssid_time.csv", "w", 'utf_8_sig')
    winsize = csv.reader(fil_winsize)
    # write_ssid = csv.writer(fil6)
except Exception:
    print "winsize_filelist open failed"
    exit()
ratio = 1000
iw_file_list = []
pre = '/home/oneT/data/'
for i in winsize:
    tmp = i[0] + '/split/'
    # print tmp
    res = os.listdir(pre + tmp)
    # print res
    for j in res:
        if j.find('iw.csv') > 0:
            jj = tmp + j
            iw_file_list.append(jj)
for xx in iw_file_list:
    # print i
    iw_f = pre + xx
    k = xx.replace('/', '_')
    k = k.replace('iw.csv', 'ratio.csv')
    re_f = pre + 'new/' + k
    qu_f = iw_f.replace('iw', 'queue')
    be_f = iw_f.replace('iw', 'beacon')
    dr_f = iw_f.replace('iw', 'dropped')
    wr_f = re_f.replace('ratio', 'stats')
    print iw_f, re_f, qu_f, be_f, dr_f, wr_f
    # break
    opend = []
    try:
        re_file = open(re_f, 'rb')
        re_r = csv.reader(re_file)
    except Exception:
        print 'ratio file open failed'
        continue
    try:
        iw_file = open(iw_f, 'rb')
        iw_r = csv.reader(iw_file)
    except Exception:
        print 'iw file open failed'
        continue
    try:
        qu_file = open(qu_f, 'rb')
        qu_r = csv.reader(qu_file)
    except Exception:
        print 'queue file open failed'
        continue
    try:
        be_file = open(be_f, 'rb')
        be_r = csv.reader(be_file)
    except Exception:
        print 'beacon file open failed'
        continue
    try:
        wr_file = open(wr_f, 'wb')
        wr_w = csv.writer(wr_file)
    except Exception:
        print 'write file open failed'
        continue
    try:
        dr_file = open(dr_f, 'rb')
        dr_r = csv.reader(dr_file)
        opend.append('drop')
    except Exception:
        print "None dropped_processed"

    opend = set(opend)
    retrans_ratio = {}

    if 'drop' not in opend:
        dropped = []
    for i in re_r:
        index = int(i[0])
        tmp = float(i[1])
        retrans_ratio[index] = [tmp]
    if re_file:
        re_file.close()

    last_duration = -1
    last_list = []

    tmp_list = []
    for i in iw_r:
        (time, station, mac_addr, device, inactive_time,
            rx_bytes, rx_packets, tx_bytes, tx_packets, tx_retries,
         tx_failed, signel, signal_avg, noise, tx_bitrate,
         rx_bitrate, active_time, busy_time,
         receive_time, transmit_time) = i
        try:
            time = int(time)
        except Exception:
            continue
        active_time = int(active_time)
        busy_time = int(busy_time)
        receive_time = int(receive_time)
        transmit_time = int(transmit_time)
        tmp = [time, station, active_time,
               busy_time, receive_time, transmit_time]
        tmp_list.append(tmp)
    if iw_file:
        iw_file.close()
    del iw_r
    gc.collect()
    iw_r = sorted(tmp_list)

    for i in iw_r:
        (time, station, active_time, busy_time,
            receive_time, transmit_time) = i
        try:
            time = int(time)
        except Exception:
            continue
        time_origin = time
        time = int(time / ratio)

        if last_duration != time:
            last_duration = time
            # process last_list
            # print last_list
            last_list = sorted(last_list)
            len_tmp = len(last_list)
            # print len_tmp, 'abc'
            if len_tmp > 1:
                # print "xxx"
                stations = set()
                (time_origin_b, station_b, active_time_b, busy_time_b,
                 receive_time_b, transmit_time_b) = last_list[0]
                (time_origin_e, station_e, active_time_e, busy_time_e,
                 receive_time_e, transmit_time_e) = last_list[len_tmp - 1]
                duration = int(time_origin_e) - int(time_origin_b)
                duration_survey = int(active_time_e) - int(active_time_b)
                duration = float(duration)
                duration_survey = float(duration_survey)
                if (duration > 0.0) and (duration_survey > 0.0):
                    for j in last_list:
                        tmp = j[1]
                        stations.add(tmp)
                    station_amount = len(stations)
                    busy_time_e = int(busy_time_e)
                    busy_time_b = int(busy_time_b)
                    receive_time_e = int(receive_time_e)
                    receive_time_b = int(receive_time_b)
                    transmit_time_b = int(transmit_time_b)
                    transmit_time_e = int(transmit_time_e)
                    busy = float(busy_time_e - busy_time_b) / duration_survey
                    recv = float(receive_time_e - receive_time_b) / \
                        duration_survey
                    tran = float(transmit_time_e -
                                 transmit_time_b) / duration_survey
                    busy = round(busy, 4)
                    recv = round(recv, 4)
                    tran = round(tran, 4)
                    try:
                        retrans_ratio[time] = retrans_ratio[time] + \
                            [station_amount, busy, recv, tran]
                    except Exception:
                        pass
            last_list = []
        last_list.append([time_origin, station, active_time, busy_time,
                          receive_time, transmit_time])

    dic_tmp = {}
    for key in retrans_ratio.keys():
        if len(retrans_ratio[key]) == 5:
            dic_tmp[key] = retrans_ratio[key]
    del retrans_ratio, last_list, last_duration, iw_r
    gc.collect()
    retrans_ratio = dic_tmp

    last_duration = -1
    last_list = []
    del tmp_list
    gc.collect()
    tmp_list = []
    for i in qu_r:
        (mac_addr, time, queue_id, bytes1,
         packets, qlen, backlog, drops, requeues) = i
        try:
            time = int(time)
        except Exception:
            continue
        time = int(time)
        queue_id = int(queue_id)
        if queue_id != 3:
            continue
        x = [time, mac_addr, bytes1,
             packets, qlen, backlog, drops, requeues]
        tmp_list.append(x)
    if qu_file:
        qu_file.close()
    del qu_r
    gc.collect()
    qu_r = sorted(tmp_list)
    for i in qu_r:
        (time, mac_addr, bytes1,
         packets, qlen, backlog, drops, requeues) = i
        try:
            time = int(time)
        except Exception:
            continue
        time = int(time)
        time_origin = time
        time = time / ratio
        time = int(time)

        if time != last_duration:
            last_duration = time
            len_tmp = len(last_list)
            if len_tmp > 1:
                (time_b, bytes_b, packets_b, qlen_b,
                 backlog_b, drops_b, requeues_b) = last_list[0]
                (time_e, bytes_e, packets_e, qlen_e,
                 backlog_e, drops_e, requeues_e) = last_list[len_tmp - 1]
                duration = int(time_e) - int(time_b)
                duration = float(duration)
                # print duration
                if duration > 0.0:
                    x = [bytes_b, packets_b, qlen_b,
                         backlog_b, drops_b, requeues_b]
                    y = [bytes_e, packets_e, qlen_e,
                         backlog_e, drops_e, requeues_e]
                    z = [0.0 for a in range(len(x))]
                    for j in range(0, len(x)):
                        x[j] = float(x[j])
                        y[j] = float(y[j])
                        z[j] = round((y[j] - x[j]) / duration, 4)
                    try:
                        if len(retrans_ratio[time]) == 5:
                            retrans_ratio[time] += z
                        elif len(retrans_ratio[time]) > 5:
                            print retrans_ratio[time], len(retrans_ratio[time])
                    except Exception:
                        pass
            last_list = []
        last_list.append([time_origin, bytes1, packets,
                          qlen, backlog, drops, requeues])
    del qu_r, tmp_list
    gc.collect()
    # print 'len before', len(retrans_ratio.keys())
    dic_tmp = {}
    for key in retrans_ratio.keys():
        if len(retrans_ratio[key]) == 11:
            dic_tmp[key] = retrans_ratio[key]
    del retrans_ratio
    gc.collect()
    retrans_ratio = dic_tmp
    # print 'len after', len(retrans_ratio.keys())
    if qu_file:
        qu_file.close()
    del dic_tmp, last_duration, last_list
    gc.collect()
    last_duration = -1
    last_list = []
    tmp_list = []
    for i in be_r:
        (time, timestamp, data_rate, current_channel,
         channel_type, ssl_signal, bw, bssid, essid, mac_addr) = i
        try:
            time = int(time)
        except Exception:
            continue
        time = int(time)
        x = [time, bssid]
        tmp_list.append(x)

    if be_file:
        be_file.close()
    del be_r
    gc.collect()
    be_r = sorted(tmp_list)
    for i in be_r:
        (time, bssid) = i
        try:
            time = int(time)
        except Exception:
            continue
        time_origin = time
        time = time / ratio
        time = int(time)

        if last_duration != time:
            last_duration = time
            bssid_set = set()
            for j in last_list:
                bssid_set.add(j[1])
            amount = len(bssid_set)
            try:
                if len(retrans_ratio[time]) == 11:
                    retrans_ratio[time] += [amount]
                elif len(retrans_ratio[time]) > 11:
                    print "xbc", retrans_ratio[time], len(retrans_ratio[time])
            except Exception:
                pass
            last_list = []

        last_list.append([time_origin, bssid])
    for key in retrans_ratio.keys():
        if len(retrans_ratio[key]) == 11:
            retrans_ratio[key] += [0]
    tmp_dic = {}
    for key in retrans_ratio.keys():
        if len(retrans_ratio[key]) == 12:
            tmp_dic[key] = retrans_ratio[key]
    del retrans_ratio
    gc.collect()
    retrans_ratio = tmp_dic
    del last_duration, last_list, be_r, tmp_list, tmp_dic
    gc.collect()
    tmp_dic = {}
    if 'drop' in opend:
        print "abc"
        tmp_list = []
        for i in dr_r:
            (ip_src, ip_dst, mac_addr, port_src, port_dst,
             drop_location, sequence, ack_sequence, time) = i
            try:
                time = int(time)
            except Exception:
                continue
            time = int(time) / ratio
            time = int(time)
            tmp_list.append(time)
        tmp_list = sorted(tmp_list)

        for i in tmp_list:
            try:
                tmp_dic[i] += 1
            except Exception:
                tmp_dic[i] = 1
        # print tmp_dic
    for key in retrans_ratio.keys():
        if len(retrans_ratio[key]) == 12:
            tmp = 0
            try:
                tmp = tmp_dic[key]
            except Exception:
                tmp = 0
            retrans_ratio[key] += [tmp]
        if len(retrans_ratio[key]) == 13:
            wr_w.writerow(retrans_ratio[key])
        elif len(retrans_ratio[key]) > 13:
            print len(retrans_ratio[key]), retrans_ratio[key]
    if wr_file:
        wr_file.close()
    # print wr_f
    # exit()
if fil_winsize:
    fil_winsize.close()
