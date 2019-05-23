from socket import socket, AF_INET, SOCK_STREAM


class Mysocket:
    def __init__(self, addr=None, port=None):
        self.remoteAddr = addr
        self.remotePort = port
        self.sock = socket(AF_INET, SOCK_STREAM)

    def init(self):
        self.sock.connect((self.remoteAddr, self.remotePort))

    def recv(self, len):
        return int(self.sock.recv(len).decode())

    def send(self, cmd):
        self.sock.send(cmd.encode())
