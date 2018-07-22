import re
import numpy as np
import matplotlib.pyplot as plt
import gc
# import statsmodels.api as sm
import MySQLdb
import csv
con = MySQLdb.Connection(host="localhost", user="root",
                         passwd="ling", port=3306)
cur = con.cursor()
con.select_db('record217')

try:
    fil1 = file("sortedrawnoiw.csv", "rb")
    fil2 = file("571anoiw.csv", "wb")
    fil3 = file("idandtime.csv", "rb")
except:
    print "tranningdata open failed"
    exit()
lines = csv.reader(fil1)
lines = list(lines)
writer1 = csv.writer(fil2)
lineall = csv.reader(fil3)
lineall = list(lineall)
tmp = lines[0]
duration = 1000
timeb = int(tmp[1]) / duration * duration
tmp = lines[len(lines) - 1]
timee = int(tmp[1]) / duration * duration + duration
# print timeb, timee
tmp = (timee - timeb) / duration
# print tmp
data_r = [0 for col in range(0, tmp)]
data = []
retran = []
all_packets = []
channel_busy = []
data_a = [0.0 for col in range(0, tmp)]
zero = 0

for line in lines:
    (x, tm, y, z, u) = line
    if int(y) != 1:
        # print "y is not 1"
        continue
    tmp = (int(tm) - timeb) / duration
    data_r[tmp] += 1
for line in lineall:
    (xid, tm) = line
    tm = (int(tm) - timeb) / duration
    try:
        data_a[tm] += 1
    except:
        # print line
        pass
del lines, lineall

for i in range(0, len(data_r)):
    ts = i * duration + timeb
    te = ts + duration
    queue_status_e = {}
    queue_status_s = {}
    sql_queue = "select * from queue where queue_id = 3 \
    and time >= %u and time < %u and \
    mac_ddr = \"04:a1:51:a3:57:1a\";" % (ts, te)
    cur.execute(sql_queue)
    results_queue = cur.fetchall()
    sql_iw = "select station, tx_bytes, time, active_time, \
    busy_time, receive_time, transmit_time, noise, \
    rx_bytes, rx_packets, tx_packets, tx_retries, tx_failed \
    from iw where mac_addr = \"04:a1:51:a3:57:1a\" and time < %d \
     and time >= %d and id > 4581;" % (te, ts)
    cur.execute(sql_iw)
    # print sql_iw, sql_queue
    results = cur.fetchall()
    if len(results) > 1 and len(results_queue) > 1:
        noise = 0.0
        rate_stations = {}
        rate_statione = {}
        (station, tx_bytes1, time1, active_time1, busy_time1, receive_time1,
         transmit_time1, noise1, rx_bytes1, rx_packets1,
         tx_packets1, tx_retries1, tx_failed1) = results[0]
        (station, tx_bytes2, time2, active_time2, busy_time2, receive_time2,
         transmit_time2, noise2, rx_bytes2, rx_packets2,
         tx_packets2, tx_retries2, tx_failed2) = results[len(results) - 1]
        tmp = float(time2 - time1) * 0.001
        if tmp == 0:
            continue
        for m in results:
            (station, tx_bytes, b, c, d, d,
             f, g, rx_bytes, rx_packets, tx_packets, tx_retries, tx_failed) = m
            noise += float(g)
            x = (tx_bytes, rx_bytes, rx_packets,
                 tx_packets, tx_retries, tx_failed)
            if station not in rate_stations.keys():
                rate_stations[station] = x
            rate_statione[station] = x
        txrate = 0.0
        rxrate = 0.0
        rxpacket = 0.0
        txretry = 0.0
        tx_fail = 0.0
        tx_packet = 0.0
        # print rate_statione
        for key in rate_statione.keys():
            txrate += float(rate_statione[key]
                            [0] - rate_stations[key][0]) / tmp
            rxrate += float(rate_statione[key]
                            [1] - rate_stations[key][1]) / tmp
            rxpacket += float(rate_statione[key]
                              [2] - rate_stations[key][2]) / tmp
            txretry += float(rate_statione[key]
                             [4] - rate_stations[key][4]) / tmp
            tx_fail += float(rate_statione[key]
                             [5] - rate_stations[key][5]) / tmp
            tx_packet += float(rate_statione[key]
                               [3] - rate_stations[key][3]) / tmp

        txrate = txrate / 1000.0
        noise = noise / float(len(results))
        del rate_statione, rate_stations
        if tmp == 0.0:
            continue
        active_time = float(active_time2 - active_time1) / tmp
        busy_time = float(busy_time2 - busy_time1) / tmp
        if data_a[i] == 0:
            print "here", data_r[i]
            continue
        try:
            all_packets.append(float(data_r[i]) / float(data_a[i]))
            if float(data_r[i]) / float(data_a[i]) > 1:
                print i
        except:
            continue

        transmit_time = float(transmit_time2 - transmit_time1) / tmp
        receive_time = float(receive_time2 - receive_time1) / tmp
        tmp_amount = 0.0
        qlen = 0.0
        qbacklog = 0.0
        for j in results_queue:
            (xid0, mac_addr1, time2, queue_id3, bytes4, packets5,
                qlen6, backlog7, drops8, requeues9) = j
            qlen += qlen6
            qbacklog += backlog7
            tmp_amount += 1.0

        qlen = qlen / tmp_amount
        qbacklog = qbacklog / tmp_amount
        (qlen_keepq, backlog_keepq, bytes_keepq, packets_keepq,
            drops_keepq, requeues_keepq,) = (0.0, 0.0, 0.0, 0.0, 0.0, 0.0)
        tmp1 = results_queue[0]
        tmp2 = results_queue[len(results_queue) - 1]
        # print tmp1, tmp2
        # exit()
        dua = float(tmp2[2] - tmp1[2])
        bytes_keepq = float(tmp2[4] - tmp1[4]) / dua * duration
        packets_keepq = float(tmp2[5] - tmp1[5]) / dua * duration
        drops_keepq = float(tmp2[8] - tmp1[8]) / dua * duration
        requeues_keepq = float(tmp2[9] - tmp1[9]) / dua * duration

        x = (float(data_r[i]), txrate,
             active_time, busy_time,
             transmit_time, receive_time, noise,
             rxrate, rxpacket, tx_packet, txretry, tx_fail, bytes_keepq,
             packets_keepq, qlen_keepq, backlog_keepq,
             drops_keepq, requeues_keepq)
        data.append(x)
        # print x
        channel_busy.append(busy_time)
        all_packets.append(data_a[i])
    else:
        # print "ss"
        zero += 1
        continue
    # exit()
    #     x = (0, 0, 0, 0, 0, 0, 0)
    #     channel_busy.append(0.0)
    #     all_packets.append(0.0)
    #     data.append(x)
    # data.append(x)
    # channel_busy.append(busy_time)
    # all_packets.append(data_a[i])
writer1.writerows(data)
print zero, "here"
# data = np.matrix(data).T
print len(data_r)
print len(channel_busy)
del data, data_a, data_r
# fig, (left_axis1) = plt.subplots(nrows=1, ncols=1, figsize=(18.0, 12.0))
# right_axis1 = left_axis1.twinx()

# p1, = left_axis1.plot(all_packets, 'b.-')
# p2, = right_axis1.plot(channel_busy, 'r.-')

# # left_axis1.set_xlim(min(all_packets), max(all_packets))
# # left_axis1.set_xticks(np.arange(min(all), max(timer), 100))

# left_axis1.set_ylim(min(all_packets), max(all_packets))
# left_axis1.set_yticks(np.arange(0, max(all_packets), 6000))

# right_axis1.set_ylim(0, 2 * max(channel_busy))
# right_axis1.set_yticks(
#     np.arange(0, 2 * max(channel_busy), 400))

# left_axis1.set_xlabel('Time (s)', fontsize=14)
# left_axis1.set_ylabel('TCP data rate (data structs / s', fontsize=14)
# right_axis1.set_ylabel('Channel busy time (ms / s)', fontsize=14)
# # plt.rc('left_axis1', labelsize=9)
# # plt.rc('right_axis1', labelsize=9)
# left_axis1.yaxis.label.set_color(p1.get_color())
# right_axis1.yaxis.label.set_color(p2.get_color())

# tkw = dict(size=5, width=1.5)
# left_axis1.tick_params(axis='y', colors=p1.get_color(), **tkw)
# right_axis1.tick_params(axis='y', colors=p2.get_color(), **tkw)
# left_axis1.tick_params(axis='x', **tkw)
# plt.legend([p1, p2], ["Retrans rate", "Channel busy time"], fontsize=14)

# plt.tight_layout(pad=0.1, h_pad=None, w_pad=None, rect=None)
# name = "E:\\linux\\workingbak\\new\\rate"
# print name
# pic = name + ".eps"
# plt.savefig(pic)
# plt.show()

del timeb, timee, channel_busy, retran
if fil1:
    fil1.close()
# if fil2:
#     fil2.close()
gc.collect()
