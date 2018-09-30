
import MySQLdb
import gc
import time
import threading
import csv
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
con = MySQLdb.Connection(host="localhost", user="root",
                         passwd="ling", port=3306)
cur = con.cursor()
con.select_db('record721')

sql = "select qlen, retrans from statistics where qlen > 0;"

cur.execute(sql)
result1 = cur.fetchall()
data = []
max_x = 0
min_x = 1000
for i in result1:
    (x, y) = i
    x = int(x + 1)
    y = int(y)
    if x > max_x:
        max_x = x
    if x < min_x:
        min_x = x
    data.append([x, y])
min_x = int(min_x)
max_x = int(max_x + 1)
data_x = [i for i in range(min_x, max_x)]
data_y = [0 for i in range(min_x, max_x)]
data_z = [0 for i in range(min_x, max_x)]
for i in data:
    (x, y) = i
    data_y[x - min_x] += y
    data_z[x - min_x] += 1
wait_del = []
for i in range(0, len(data_y)):
    if data_z[i] == 0:
        wait_del.append(i)
        continue
    tmp = float(data_y[i]) / float(data_z[i])
    tmp = round(tmp, 4)
    data_y[i] = tmp
wait_del = sorted(wait_del, reverse=True)
for i in wait_del:
    del data_x[i], data_y[i]
plt.plot(data_x, data_y)
plt.show()
if con:
    con.close()
del result1
gc.collect()
