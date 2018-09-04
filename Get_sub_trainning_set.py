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
    rat.append(ratio)
a = len(rat) / 4
rat = sorted(rat)
print rat[a], rat[2 * a], rat[3 * a]

if file:
    file.close()
del rat
gc.collect()
