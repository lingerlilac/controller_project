# -*- coding: utf-8 -*-
"""
Created on Mon Nov 28 15:05:36 2016
#这个文件用来对预处理之后的文件pre.csv进行机器学习计算，分类结果写入pdf文件，之后计算准确度、召回率、f1值
@author: WangYixin
"""
import pandas as pd
from pandas import Series
import numpy as np
import pydotplus
from sklearn import tree
from sklearn.externals.six import StringIO
from sklearn.metrics import precision_recall_curve
from sklearn.metrics import classification_report
from sklearn.model_selection import train_test_split
from sklearn.metrics import precision_recall_fscore_support as score
import csv
# load data
try:
    fil1 = open("modlist2.csv", "rb")
    results = csv.reader(fil1)
except:
    print "statistics_data.csv open failed"
    exit()
# mydata = list(results)
xdata = []
ydata = []
for i in results:
    ydata.append(i[0])
    xdata.append(i[1:len(i)])

print "1"
# min_sample_split将划分进行到底，如果设置是默认的2的时候将会有一部分划分不准确，但是这样做有过拟合
# 的风险
clf = tree.DecisionTreeClassifier(
    criterion='entropy', min_samples_split=200, min_samples_leaf=400)  # 信息熵作为划分的标准，CART
x_train, x_test, y_train, y_test = train_test_split(
    xdata, ydata, test_size=0.3)
clf = clf.fit(x_train, y_train)
print "2"
dot_data = StringIO()
print "3"
parameters = ["Stations", "Channel utility", "Receive utility",
              "Transmit utility", "Bytes",
              "Packets", "Qlen", "Backlog", "Drops in queue",
              "Requeues", "Neighbors", "Drops in mac"]
tree.export_graphviz(
    clf, out_file=dot_data,
    feature_names=parameters,
    class_names=['0', '1'],
    filled=True, rounded=True, special_characters=True)
print "4"
graph = pydotplus.graph_from_dot_data(dot_data.getvalue())
print "here"
# 导出
# graph.write_pdf('sport.pdf')
# graph.write_png('sport.png')

print len(clf.feature_importances_), len(parameters)
# exit()
for i in range(0, len(parameters)):
    print parameters[i], ':', clf.feature_importances_[i]

precision, recall, fscore, support = score(y_train, clf.predict(x_train))
print('precision: {}'.format(precision))
print('recall: {}'.format(recall))
print('fscore: {}'.format(fscore))
print('support: {}'.format(support))
