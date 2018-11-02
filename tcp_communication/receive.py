#-*- coding: UTF-8 -*-
import socket
import time
import SocketServer
import struct
import os
import thread
import csv
host = '192.168.11.101'
port = 12307
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # 定义socket类型
s.bind((host, port))  # 绑定需要监听的Ip和端口号，tuple格式
s.listen(1)


def conn_thread(connection, address, f_tmp, f_w):
    while True:
        try:

            # print filename
            buf = connection.recv(27)
            # f_w.writerow([buf])
            # print buf
            # connection.settimeout(600)
            # fileinfo_size = struct.calcsize('128sl')
            # buf = connection.recv(fileinfo_size)
            # if buf:  # 如果不加这个if，第一个文件传输完成后会自动走到下一句
            #     print struct.unpack('128sl', buf)
            #     exit()
            #     filename, filesize = struct.unpack('128sl', buf)
            #     filename_f = filename.strip('\00')
            #     filenewname = os.path.join('/home/lin/', ('new_' + filename_f))
            #     print 'file new name is %s, filesize is %s' % (filenewname, filesize)
            #     recvd_size = 0  # 定义接收了的文件大小
            #     file = open(filenewname, 'wb')
            #     print 'stat receiving...'
            #     while not recvd_size == filesize:
            #         if filesize - recvd_size > 1024:
            #             rdata = connection.recv(1024)
            #             recvd_size += len(rdata)
            #         else:
            #             rdata = connection.recv(filesize - recvd_size)
            #             recvd_size = filesize
            #         file.write(rdata)
            #     file.close()
            #     print 'receive done'
                # connection.close()
        except socket.timeout:
            connection.close()
    print "fk"
    if f_tmp:
        f_tmp.close()


while True:
    connection, address = s.accept()
    print('Connected by ', address)
    # thread = threading.Thread(target=conn_thread,args=(connection,address)) #使用threading也可以
    # thread.start()
    
    (addr, port) = address
    filename = addr + '_' + str(port) + '.csv'

    try:
        f_tmp = open(filename, 'wb')
        f_w = csv.writer(f_tmp)
    except:
        print filename, 'openfailed'
        exit()
    thread.start_new_thread(conn_thread, (connection, address, f_tmp, f_w))
s.close()
