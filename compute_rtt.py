#-*—coding:utf8-*-
import numpy as np
import gc
import re
import csv
import codecs
from decimal import *
import os
import statsmodels.api as sm
import matplotlib
import matplotlib.pyplot as plt
try:
    fil_winsize = codecs.open("720winsize.txt", "r", 'utf_8_sig')
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
    rtt_down_record_rtt = []
    rtt_down_record_sequence = []
    rtt_down = {}
    rtt_up = {}
    try:
        f_tmp = open(f, 'rb')
        f_w = csv.reader(f_tmp)
        x = f.replace('winsize', 'rtt')
        rtt_f = open(x, 'wb')
        x_w = csv.writer(rtt_f)
    except Exception:
        print f_tmp, 'open failed'
    for i in f_w:
        # print i
        break
    sequence_dic = {}
    # print "abc"
    for i in f_w:
        (mac_addr, eth_src, eth_dst, ip_src, ip_dst,
            srcport, dstport, sequence, ack_sequence,
         windowsize, cal_windowsize, time,
         datalength, flags, kind, length, wscale) = i
        # del i
        # srcport = np.uint32(srcport)
        # dstport = np.uint32(dstport)
        # ip_src = re.sub(".", "", ip_src)
        # ip_dst = re.sub(".", "", ip_dst)
        ip_src = ip_src.replace(".", "")
        ip_dst = ip_dst.replace(".", "")
        sequence = np.uint32(sequence)
        ack_sequence = np.uint32(ack_sequence)
        time = np.uint64(time)
        datalength = np.uint32(datalength)
        # print ip_src
        if ip_src.find("1921681") == -1:
            # print i
            # print "here"
            state_sequence_exit = -1
            ack_sequence_exit = -1
            try:
                rtt_down[mac_addr + ip_src +
                         str(srcport) + str(sequence)][5] += 1
                state_sequence_exit = rtt_down[
                    mac_addr + ip_src + str(srcport) + str(sequence)][5]
                # print "there"
            except Exception:
                if datalength != 0:
                    rtt_down[
                        mac_addr + ip_src +
                        str(srcport) + str(sequence + datalength)] = [
                        time, datalength, 0, 0, 0, 0]
                else:
                    pass
            try:
                ack_sequence_exit = rtt_down[
                    mac_addr + ip_src + str(srcport) + str(sequence)][4]
                # print "there"
            except Exception:
                pass
            # print ack_sequence_exit, state_sequence_exit
            if ack_sequence_exit > 0 and state_sequence_exit == 1:
                # print xid, sequence, ack_sequence
                try:
                    wifirtt = 0
                    wifirtt = time - \
                        rtt_down[mac_addr + ip_src +
                                 str(srcport) + str(sequence)][0]
                    # print xid, sequence, ack_sequence
                    # sql_1 = "select id from winsize where \
                    # (sequence + datalength) = %d and id < %d and \
                    # id > (%d - 10000);" % (sequence, xid, xid)
                    # cur.execute(sql_1)
                    # back_results = cur.fetchall()
                    # xid1 = -1
                    # try:
                    #     xid1 = back_results[0][0]
                    #     if xid != -1:
                    #         sql_2 = "select * from winsize \
                    # where id <= (%d + 0)\
                    #          and id >= (%d -0);" % (xid, xid1)
                    #         cur.execute(sql_2)
                    #         back_results = cur.fetchall()
                    #         print "----------------------\
                    # ------------------------------------------------------"
                    #         for j in back_results:
                    #             print j
                    # except:
                    #     pass
                    if wifirtt < 4000:
                        rtt_down_record_rtt.append(wifirtt)
                    del rtt_down[mac_addr + ip_src +
                                 str(srcport) + str(sequence)]
                except Exception:
                    pass
        else:
            # print i
            try:
                rtt_down[mac_addr +
                         ip_dst + str(dstport) + str(ack_sequence)][4] += 1
                # print "here333"
            except Exception:
                pass
        # tmp = -1
        # try:
        #     rtt_down[mac_addr + ip_src +
        #              str(srcport) + str(sequence)][5] += 1
        #     tmp = rtt_down[mac_addr + ip_src +
        #                    str(srcport) + str(sequence)][5]
        # except:
        #     if datalength != 0:
        #         rtt_down[mac_addr + ip_src + str(srcport) +
        #                  str(sequence +
        #                      datalength)] = [time, datalength, 0, 0, 0, 0]
        #     else:
        #         pass
        # tmp1 = -1
        # try:
        #     tmp1 = rtt_down[mac_addr + ip_dst +
        #                     str(dstport) + str(ack_sequence)][4]
        # except:
        #     pass
        # if tmp == 1 and tmp1 == 1:
        #     try:
        #         wifirtt = 0
        #         wifirtt = time - \
        #             rtt_down[mac_addr + ip_src +
        #                      str(srcport) + str(sequence)][0]
        #         print xid, sequence, ack_sequence
        #         sql_1 = "select id from winsize where \
        #         (sequence + datalength) = %d and id < %d and \
        #         id > (%d - 10000);" % (sequence, xid, xid)
        #         cur.execute(sql_1)
        #         back_results = cur.fetchall()
        #         xid1 = -1
        #         try:
        #             xid1 = back_results[0][0]
        #             if xid != -1:
        #                 sql_2 = "select * from winsize \
        #         where id <= (%d + 0)\
        #                  and id >= (%d -0);" % (xid, xid1)
        #                 cur.execute(sql_2)
        #                 back_results = cur.fetchall()
        #                 print "----------------------\
        #         ------------------------------------------------------"
        #                 for j in back_results:
        #                     print j
        #         except:
        #             pass
        #         if wifirtt < 10000:
        #             rtt_down_record_rtt.append(wifirtt)
        #         del rtt_down[mac_addr + ip_dst +
        #                      dstport + str(ack_sequence)]
        #     except:
        #         pass
        # else:
        #     pass

        # try:
        #     rtt_down[mac_addr + ip_dst +
        #              str(dstport) + str(ack_sequence)][4] += 1
        # except:
        #     pass
    if f_tmp:
        f_tmp.close()
    # break
    del rtt_down
    tmp = 0.0
    length = float(len(rtt_down_record_rtt))
    for i in rtt_down_record_rtt:
        x_w.writerow([i])
        tmp += float(i) / length
    if rtt_f:
        rtt_f.close()
    avage_rtt = tmp
    # avage_rtt = sum(float(rtt_down_record_rtt) / float(len(rtt_down_record_rtt)))
    avage_rtt = round(avage_rtt, 2)
    avage_rtt = "Avg of RTT: " + str(avage_rtt)
    ecdf = sm.distributions.ECDF(rtt_down_record_rtt)
    x = np.linspace(min(rtt_down_record_rtt), max(rtt_down_record_rtt), 500)
    y = ecdf(x)
    # matplotlib.rc('xtick', labelsize=20)
    # matplotlib.rc('ytick', labelsize=20)
    font = {'size': 20}
    # print rtt_down_record_rtt
    matplotlib.rc('font', **font)
    plt.figure(1)
    # x = np.linspace(0, count, count)
    # plt.xlim(rtt_down_record_sequence[0], rtt_down_record_sequence[
    # len(rtt_down_record_sequence) - 1])
    # plt.ylim(0, count)
    # plt.xlim(0, 1000)
    plt.xlim(min(x), max(x))
    plt.xlabel('time (ms)')
    plt.ylabel('CDF')
    # plt.gca().set_xscale('log')
    # plt.plot(rtt_down_record_sequence, rtt_down_record_rtt, label=avage_rtt)
    plt.plot(x, y, label=avage_rtt)
    legend = plt.legend(loc='lower right', fontsize=20)
    # legend.get_title().set_fontsize(fontsize=40)
    # plt.show()
    pic_name = f.replace('winsize.csv', 'pic.jpg')
    plt.savefig(pic_name)
    plt.close()

    del rtt_down_record_rtt, rtt_down_record_sequence, rtt_up
    # break
if fil_winsize:
    fil_winsize.close()
