# Copyright (C) 2011 Nippon Telegraph and Telephone Corporation.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from ryu.base import app_manager
from ryu.controller import ofp_event
from ryu.controller.handler import CONFIG_DISPATCHER, MAIN_DISPATCHER
from ryu.controller.handler import set_ev_cls
from ryu.ofproto import ofproto_v1_3
from ryu.lib.packet import packet
from ryu.lib.packet import ethernet
from ryu.lib.packet import ether_types
import struct
from ryu import utils
from ryu.ofproto import ether
from ryu.ofproto import inet
from ryu.ofproto.ofproto_v1_2 import OFPG_ANY
from ryu.lib import hub

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
import re
import ctypes
import subprocess
import threading


class SimpleSwitch13(app_manager.RyuApp):
    OFP_VERSIONS = [ofproto_v1_3.OFP_VERSION]

    def __init__(self, *args, **kwargs):
        super(SimpleSwitch13, self).__init__(*args, **kwargs)
        self.mac_to_port = {}
        # self.thread_list = []
        # for i in xrange(5):
        #     print i, "here"
        #     sthread = threading.Thread(
        #         target=self._get_realtime_data)
        #     sthread.start()
        #     self.thread_list.append(sthread)
        # print "after spawn"

    # def _get_realtime_data(self):
    #     ll = ctypes.cdll.LoadLibrary
    #     lib = ll("/home/lin/controller_project/receivea.so")
    #     # lib.fack(number)

    @set_ev_cls(ofp_event.EventOFPSwitchFeatures, CONFIG_DISPATCHER)
    def switch_features_handler(self, ev):
        datapath = ev.msg.datapath
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser

        # install table-miss flow entry
        #
        # We specify NO BUFFER to max_len of the output action due to
        # OVS bug. At this moment, if we specify a lesser number, e.g.,
        # 128, OVS will send Packet-In with invalid buffer_id and
        # truncated packet data. In that case, we cannot output packets
        # correctly.  The bug has been fixed in OVS v2.1.0.
        match = parser.OFPMatch()
        actions = [parser.OFPActionOutput(ofproto.OFPP_CONTROLLER,
                                          ofproto.OFPCML_NO_BUFFER)]
        self.add_flow(datapath, 0, match, actions)
        print "switch_features_handler"

    def add_flow(self, datapath, priority, match, actions, buffer_id=None):
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser

        inst = [parser.OFPInstructionActions(ofproto.OFPIT_APPLY_ACTIONS,
                                             actions)]
        if buffer_id:
            mod = parser.OFPFlowMod(datapath=datapath, buffer_id=buffer_id,
                                    priority=priority, match=match,
                                    instructions=inst)
        else:
            mod = parser.OFPFlowMod(datapath=datapath, priority=priority,
                                    match=match, instructions=inst)
        datapath.send_msg(mod)

    @set_ev_cls(ofp_event.EventOFPPacketIn, MAIN_DISPATCHER)
    def _packet_in_handler(self, ev):
        # If you hit this you might want to increase
        # the "miss_send_length" of your switch
        if ev.msg.msg_len < ev.msg.total_len:
            self.logger.debug("packet truncated: only %s of %s bytes",
                              ev.msg.msg_len, ev.msg.total_len)
        msg = ev.msg
        datapath = msg.datapath
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser
        in_port = msg.match['in_port']

        pkt = packet.Packet(msg.data)
        eth = pkt.get_protocols(ethernet.ethernet)[0]
        print "_packet_in_handler"
        if eth.ethertype == ether_types.ETH_TYPE_LLDP:
            # ignore lldp packet
            return
        dst = eth.dst
        src = eth.src

        dpid = datapath.id
        self.mac_to_port.setdefault(dpid, {})

        self.logger.info("packet in %s %s %s %s", dpid, src, dst, in_port)

        # learn a mac address to avoid FLOOD next time.
        self.mac_to_port[dpid][src] = in_port

        if dst in self.mac_to_port[dpid]:
            out_port = self.mac_to_port[dpid][dst]
        else:
            out_port = ofproto.OFPP_FLOOD

        actions = [parser.OFPActionOutput(out_port)]

        # install a flow to avoid packet_in next time
        if out_port != ofproto.OFPP_FLOOD:
            match = parser.OFPMatch(in_port=in_port, eth_dst=dst)
            # verify if we have a valid buffer_id, if yes avoid to send both
            # flow_mod & packet_out
            if msg.buffer_id != ofproto.OFP_NO_BUFFER:
                self.add_flow(datapath, 1, match, actions, msg.buffer_id)
                return
            else:
                self.add_flow(datapath, 1, match, actions)
        data = None
        if msg.buffer_id == ofproto.OFP_NO_BUFFER:
            data = msg.data

        out = parser.OFPPacketOut(datapath=datapath, buffer_id=msg.buffer_id,
                                  in_port=in_port, actions=actions, data=data)
        datapath.send_msg(out)
        tree = self.compute_tree()
        ip_addr = "192.168.11.102"
        port = 80
        self.install_experimental_flow(ip_addr, tree, port, ev)

    def install_experimental_flow(self, ip_addr, tree, port, ev):
        msg = ev.msg
        datapath = msg.datapath
        parser = datapath.ofproto_parser
        ofproto = datapath.ofproto
        dpiActionRTSP = parser.OFPExperimenter(
            datapath, None, None, 'AAAA8B1,2,3,4,5,6;2,2,3,4,5,6;3,2,3,4,5,6;4,2,3,4,5,6;5,2,3,4,5,6;6,2,3,4,5,6;7,2,3,4,5,6;8,2,3,4,5,6;.')
        # dpiActionRTSP.xid = msg.xid
        # print "sssss", dpiActionRTSP

        # TCP DST RTSP Test Traffic
        # ip_addr = self.ipv4_to_int(ip_addr)
        # match = self.create_match(parser, [(ofproto.OXM_OF_ETH_TYPE,
        #                                     ether.ETH_TYPE_IP),
        #                                    (ofproto.OXM_OF_IPV4_DST, ip_addr),
        #                                    (ofproto.OXM_OF_IP_PROTO, 6),
        #                                    (ofproto.OXM_OF_TCP_DST, port)])
        # action = parser.OFPInstructionActions(ofproto.OFPIT_WRITE_ACTIONS,
        #                                       [dpiActionRTSP])
        # flow_mod = self.create_flow_mod(
        #     datapath, 0, 0, 125, 0, match, [action])
        # # print flow_mod
        datapath.send_msg(dpiActionRTSP)
        print dpiActionRTSP

    def ipv4_to_str(self, integre):
        ip_list = [str((integre >> (24 - (n * 8)) & 255)) for n in range(4)]
        return '.'.join(ip_list)

    def ipv4_to_int(self, string):
        ip = string.split('.')
        assert len(ip) == 4
        i = 0
        for b in ip:
            b = int(b)
            i = (i << 8) | b
        return i

    def create_match(self, parser, fields):
        """Create OFP match struct from the list of fields."""
        match = parser.OFPMatch()
        for a in fields:
            match.append_field(*a)
        return match

    def create_flow_mod(self, datapath, idle_timeout, hard_timeout, priority,
                        table_id, match, instructions):
        """Create OFP flow mod message."""
        ofproto = datapath.ofproto
        # print datapath.id
        flow_mod = datapath.ofproto_parser.OFPFlowMod(datapath, 0, 0,
                                                      table_id,
                                                      ofproto.OFPFC_ADD,
                                                      idle_timeout,
                                                      hard_timeout, priority,
                                                      ofproto.OFPCML_NO_BUFFER,
                                                      ofproto.OFPP_ANY,
                                                      OFPG_ANY, 0,
                                                      match, instructions)
        return flow_mod

    def compute_tree(self):
        # datadic = 'statistics_data.csv'
        # mydata = pd.read_csv(datadic)

        # xdata = mydata.values[:, 1:65]
        # ydata = mydata.values[:, 0]
        # tmp = ["allpackets_1",
        #        "noise_1", "active_time_1", "busy_time_1", "receive_time_1",
        #        "transmit_time_1", "rbytes_1",
        #        "packets_1", "qlen_1", "backlogs_1",
        #        "drops_1", "requeues_1", "retrans_2",
        #        "allpackets_2", "noise_2",
        #        "active_time_2", "busy_time_2",
        #        "receive_time_2", "transmit_time_2",
        #        "rbytes_2", "packets_2", "qlen_2", "backlogs_2", "drops_2",
        #        "requeues_2", "retrans_3", "allpackets_3", "noise_3",
        #        "active_time_3", "busy_time_3",
        #        "receive_time_3", "transmit_time_3",
        #        "rbytes_3", "packets_3", "qlen_3",
        #        "backlogs_3", "drops_3", "requeues_3",
        #        "retrans_4", "allpackets_4", "noise_4",
        #        "active_time_4", "busy_time_4",
        #        "receive_time_4", "transmit_time_4",
        #        "rbytes_4", "packets_4", "qlen_4",
        #        "backlogs_4", "drops_4", "requeues_4",
        #        "retrans_5", "allpackets_5",
        #        "noise_5", "active_time_5", "busy_time_5", "receive_time_5",
        #        "transmit_time_5", "rbytes_5",
        #        "packets_5", "qlen_5", "backlogs_5",
        #        "drops_5", "requeues_5"]
        # # tmp = ("noise", "active_time",
        # #        "busy_time", "receive_time", "transmit_time",
        # #        "rbytes", "packets", "qlen",
        # #        "backlogs", "drops", "requeues")
        # print len(tmp)
        # print "1"
        # clf = tree.DecisionTreeClassifier(
        #     criterion='entropy', min_samples_split=200,
        #     min_samples_leaf=400)
        # x_train, x_test, y_train, y_test = train_test_split(
        #     xdata, ydata, test_size=0.3)
        # clf = clf.fit(x_train, y_train)
        # print "2"
        # dot_data = StringIO()
        # print "3"

        # tree.export_graphviz(
        #     clf, out_file=dot_data,
        #     feature_names=tmp,
        #     class_names=['0', '1', '2'],
        #     filled=True, rounded=True, special_characters=True)
        # print "4"
        # graph = pydotplus.graph_from_dot_data(dot_data.getvalue())
        # print "here"
        # graph.write_pdf('sport.pdf')
        # graph.write_png('sport.png')
        # graph.write('abc')
        # strtree = graph.to_string()
        # # print strtree
        # strtree = re.split("\n", strtree)
        # nodes = []
        # links = []
        # for i in strtree:
        #     # print i
        #     lief = True
        #     if i.find("&le") > 0:
        #         # print "here", i
        #         lief = False
        #     i = re.split(" ", i)
        #     try:
        #         x = int(i[0])
        #     except:
        #         continue
        #     try:
        #         (x, y, z) = (i[0], i[1], i[2])
        #         x = int(x)
        #         try:
        #             z = z.replace(";", "")
        #             z = int(z)
        #         except:
        #             pass
        #         if y == "->":
        #             links.append((x, z))
        #         else:
        #             if lief is False:
        #                 tmp = str(i)
        #                 index1 = tmp.find("=<") + 2
        #                 index2 = tmp.find("<br/>")
        #                 tmp = tmp[index1:index2]
        #                 tmp = tmp.replace("'", "")
        #                 tmp = re.split(",", tmp)
        #                 # print tmp
        #                 (left, right) = (tmp[0], tmp[2])
        #                 # right = float(right)
        #                 # print left, right
        #                 # links.append((x, z, value))
        #                 nodes.append((x, left, right))
        #             else:
        #                 nodes.append(x, "BBB", -1000)
        #     except:
        #         pass
        # print nodes
        # print links
        # # print len(clf.feature_importances_), len(tmp)
        # # exit()
        # for i in range(0, len(tmp)):
        #     print tmp[i], ':', clf.feature_importances_[i]
        # # print clf.feature_importances_
        # # print len(clf.feature_importances_)
        # # answer = clf.predict(x_train)
        # precision, recall, fscore, support = score(
        #     y_train, clf.predict(x_train))
        # print 'precision: {}'.format(precision)
        # print 'recall: {}'.format(recall)
        # print 'fscore: {}'.format(fscore)
        # print 'support: {}'.format(support)
        # links = ["s" for i in range(0, 400)]
        links = "abcdef"
        # links = str(links)
        return links

    # def tree_evolution(self):
    #     # reinforcement learning algorithm
    #     # used to make decision trees asymptotic
