#-*—coding:utf8-*-
import numpy as np
import gc
import re
import csv
import codecs
import matplotlib
import matplotlib.pyplot as plt
from decimal import *

# maclist = ("04:a1:51:96:ca:83",
#            "04:a1:51:a3:57:1a",
#            "44:94:fc:82:d9:8e",
#            "04:a1:51:a8:6e:c5",
#            "04:a1:51:96:64:cb",
#            "04:a1:51:a0:65:c0",
#            "04:a1:51:a7:54:f9",
#            "04:a1:51:8e:f1:cb")

try:
    fil2 = codecs.open("statistics.txt", "w", 'utf_8_sig')
    # fil6 = codecs.open("channel_ssid_time.csv", "w", 'utf_8_sig')
    write_record = csv.writer(fil2)
    # write_ssid = csv.writer(fil6)
except Exception:
    print "tranningdata open failed"
    exit()

try:
    fil1 = open("6667.txt", "r")
except Exception:
    print "6666 open failed."

lines = fil1.readlines()

for item in lines:
    