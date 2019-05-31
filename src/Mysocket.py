from socket import socket, AF_INET, SOCK_STREAM


class Mysocket:
    def __init__(self, addr=None, port=None):
        self.addr = addr
        self.port = port
        self.sock = socket(AF_INET, SOCK_STREAM)

    def init(self):
        self.sock.connect((self.addr, self.port))

    def bind(self):
        self.sock.bind(('', int(self.port)))

    def recv(self, len):
        return self.sock.recv(len).decode()

    def send(self, cmd):
        self.sock.send(cmd.encode())

    def getSock(self):
        return self.sock
