# SPDX-FileCopyrightText: 2023 wzyforgit
#
# SPDX-License-Identifier: GPL-3.0-or-later

from parse import JsonParse

import sys
import socketserver

class ServerHandle(socketserver.BaseRequestHandler):
    def fullDataPack(self, data):
        if self.remainingDataLength == 0 and len(self.cachedData) == 0: #缓存为空且剩余数据量为0
            self.remainingDataLength = (data[0] << 8) | data[1]
            self.cachedData = data[2:]
            if len(self.cachedData) == self.remainingDataLength: #当前包即为完整的一个包
                self.remainingDataLength = 0
                self.cachedData = b''
                return data[2:]
            else: #当前包不完整
                self.remainingDataLength = self.remainingDataLength - len(self.cachedData)
                return None

        if self.remainingDataLength != 0 and len(self.cachedData) != 0: #存在缓存且剩余数据量不为0
            if len(data) < self.remainingDataLength: #数据量完全不够
                self.remainingDataLength = self.remainingDataLength - len(data)
                self.cachedData = self.cachedData + data
                return None
            elif len(data) == self.remainingDataLength: #数据量刚好足够
                self.remainingDataLength = 0
                recvData = self.cachedData + data
                self.cachedData = b''
                return recvData
            else: #未考虑到的情况，打印错误信息后强制退出
                print('Error: Did not take this into account')
                print('remainingDataLength: %d' % len(self.remainingDataLength))
                print('len(cachedData): %d' % len(self.cachedData))
                print('len(data): %d' % len(data))
                exit(2)

    def handle(self):
        self.remainingDataLength = 0
        self.cachedData = b''
        self.parser = JsonParse('/test/tmp')
        conn = self.request
        print('connect incomming: %s' % (self.client_address, ))
        while True:
            data = conn.recv(4096)
            if data == None:
                print('connect disconnect: %s' % (self.client_address, ))
                break
            data = self.fullDataPack(data)
            if data != None:
                ret = self.parser.parseJsonFromClient(data.decode('utf-8'))
                retBin = ret.encode('utf-8')
                lenBin = len(retBin).to_bytes(2, byteorder = 'big', signed = False)
                conn.sendall(lenBin + retBin)

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: python3 server.py [ip address] [port]')
        exit(1)

    server = socketserver.ThreadingTCPServer((sys.argv[1], int(sys.argv[2])), ServerHandle)
    print('TCP server have created')
    server.serve_forever()
