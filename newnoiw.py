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
    fil3 = file("idandtime.csv", "rb")
    fil2 = file("571anoiw.csv", "wb")
except:
    print "tranningdata open failed"
    exit()
lines = csv.reader(fil1)
lines = list(lines)
lineall = csv.reader(fil3)
writer1 = csv.writer(fil2)
tmp = lines[0]
duration = 1000
timeb = int(tmp[1]) / duration * duration
tmp = lines[len(lines) - 1]
timee = int(tmp[1]) / duration * duration + duration
# print timeb, timee
tmp = (timee - timeb) / duration
# print tmp
data_r = [0 for col in range(0, tmp)]
dataall = [0.0 for col in range(0, tmp)]
data = []
for line in lines:
    (x, tm, y, z, u) = line
    if int(y) != 1:
        # print "y is not 1"
        continue
    tmp = (int(tm) - timeb) / duration
    data_r[tmp] += 1
for i in lineall:
    (a, b) = i
    tm = int(b)
    tm = (int(tm) - timeb) / duration
    try:
        dataall[tm] += 1
    except:
        pass
del lines, lineall
sql_iw = "select id, station, tx_bytes, time, active_time, \
busy_time, receive_time, transmit_time,\
 noise from iw where mac_addr = \"04:a1:51:a3:57:1a\" \
 and time >= %d and time < %d;" % (timeb, timee)
cur.execute(sql_iw)
resultall = cur.fetchall()
# print len(resultall)
# del resultall
idtotime = {}
idtoid = {}
index = 0
channel_busy_time = []
retran = []
all_packets = []
# duptimes = []
for i in resultall:
    xid = int(i[0])
    tm = i[3]
    tm = (int(tm) - timeb) / duration
    idtoid[xid] = index
    index += 1
    try:
        idtotime[tm].append(xid)
    except:
        idtotime[tm] = [xid]
# print idtotime
for i in idtotime.keys():
    ids = idtotime[i]

    noise = 0.0
    rate_stations = {}
    rate_statione = {}
    if len(ids) < 1:
        continue
    (t, station, tx_bytes1, time1, active_time1, busy_time1, receive_time1,
     transmit_time1, noise1) = resultall[idtoid[ids[0]]]
    (t, station, tx_bytes2, time2, active_time2, busy_time2, receive_time2,
     transmit_time2, noise2) = resultall[idtoid[ids[len(ids) - 1]]]
    tmp = float(time2 - time1) * 0.001

    if tmp == 0:
        continue
    for m in ids:
        (t, station, tx_bytes, b, c, d, d,
         f, g) = resultall[idtoid[m]]
        noise += float(g)
        if station not in rate_stations.keys():
            rate_stations[station] = float(tx_bytes)
        rate_statione[station] = float(tx_bytes)
    txrate = 0.0
    for key in rate_statione.keys():
        txrate += float(rate_statione[key] - rate_stations[key]) / tmp
    txrate = txrate / 1000.0
    noise = noise / float(len(ids))
    del rate_statione, rate_stations
    if tmp == 0.0:
        continue
    active_time = float(active_time2 - active_time1) / tmp
    busy_time = float(busy_time2 - busy_time1) / tmp
    if busy_time < 0:
        print t
    transmit_time = float(transmit_time2 - transmit_time1) / tmp
    receive_time = float(receive_time2 - receive_time1) / tmp
    x = (data_r[i], txrate, active_time, busy_time,
         transmit_time, receive_time, noise)
    data.append(x)
    try:
        all_packets.append(float(data_r[i]) / float(dataall[i]))
        channel_busy_time.append(float(busy_time2 - busy_time1))
    except:
        continue
    for j in ids:
        del idtoid[j]
    del idtotime[i]
# print data
writer1.writerows(data)

fig, (left_axis1) = plt.subplots(nrows=1, ncols=1, figsize=(6.0, 4.0))
right_axis1 = left_axis1.twinx()

p1, = left_axis1.plot(all_packets, 'b.-')
p2, = right_axis1.plot(channel_busy_time, 'r.-')

# left_axis1.set_xlim(min(all_packets), max(all_packets))
# left_axis1.set_xticks(np.arange(min(all), max(timer), 100))

left_axis1.set_ylim(0.0, 1.0)
left_axis1.set_yticks(np.arange(0.0, 1.0, 0.2))

right_axis1.set_ylim(0, max(channel_busy_time))
right_axis1.set_yticks(
    np.arange(0, max(channel_busy_time), 200))

left_axis1.set_xlabel('Time (s)', fontsize=14)
left_axis1.set_ylabel('TCP data rate (data structs / s', fontsize=14)
right_axis1.set_ylabel('Retransmission rate (times / s)', fontsize=14)
# plt.rc('left_axis1', labelsize=9)
# plt.rc('right_axis1', labelsize=9)
left_axis1.yaxis.label.set_color(p1.get_color())
right_axis1.yaxis.label.set_color(p2.get_color())

tkw = dict(size=5, width=1.5)
left_axis1.tick_params(axis='y', colors=p1.get_color(), **tkw)
right_axis1.tick_params(axis='y', colors=p2.get_color(), **tkw)
left_axis1.tick_params(axis='x', **tkw)
plt.legend([p1, p2], ["Retrans rate", "Channel busy time"], fontsize=14)

plt.tight_layout(pad=0.1, h_pad=None, w_pad=None, rect=None)
name = "E:\\linux\\workingbak\\new\\rate"
print name
pic = name + ".eps"
plt.savefig(pic)
plt.show()

del timeb, timee, data, dataall, all_packets
if fil1:
    fil1.close()
if fil2:
    fil2.close()
if fil3:
    fil3.close()
gc.collect()
