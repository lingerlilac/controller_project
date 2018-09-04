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
    file = open('modlist11.csv', 'rb')
    data = csv.reader(file)
except Exception:
    print "file"
rat = []
for i in data:
    try:
        ratio = i[0]
    except Exception:
        print i, "begin"
    ratio = float(ratio)
    x = i
    if ratio <= 0.0002:
        x[0] = 0
    elif (ratio > 0.0002) and (ratio <= 0.0146):
        x[0] = 1
    elif (ratio > 0.0146) and (ratio <= 0.0541):
        x[0] = 2
    elif (ratio > 0.0541):
        x[0] = 3
    # print x
    rat.append(x)
try:
    ff = open('modlist.csv', 'wb')
    rr = csv.writer(ff)
except:
    print "xx"
    exit()
print len(rat)
# exit()
rr.writerows(rat)
if ff:
    ff.close()

if file:
    file.close()
del rat
gc.collect()
