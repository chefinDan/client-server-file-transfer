import sys
import statuscodes as code
from Mysocket import Mysocket


def main():
    if len(sys.argv) != 2:
        print("*** ERROR usage: make client PORT=<port_num>")
        return 1

    sock = Mysocket('localhost', int(sys.argv[1]))
    sock.init()
    status = sock.recv(3)

    if status != code.CONNECT:
        print("CLIENT ERROR: cannot establish control connection with {} port {}\
        ".format(sock.remoteAddr, sock.remotePort))
        return -1

    print("ftp> {}\n".format(status))

    while(1):
        cmd = input("ftp> ")
        sock.send(cmd)
        status = sock.recv(3)
        print("ftp> {}\n".format(status))

# def ctrlConnect(addr, port):
#     status_len = 3
#     clientSocket = socket(AF_INET, SOCK_STREAM)
#     clientSocket.connect((addr, port))
#     res_raw = clientSocket.recv(status_len)
#     res_str = res_raw.decode()
#
#     if res_str.find(str(code.CONNECT)) == 0:
#         return code.CONNECT
#     else:
#         return -1


def prompt():
    sys.stdout.write("ftp> ")


# def dataConnect(addr, port):
#     status_len = 3
#     clientSocket = socket(AF_INET, SOCK_STREAM)
#     clientSocket.connect((addr, port))
#     res_raw = clientSocket.recv(status_len)
#     res_str = res_raw.decode()
#
#     if res_str.find(str(code.CONNECT)) == 0:
#         return code.CONNECT
#     else:
#         return -1


main()
