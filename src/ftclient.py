import sys
from socket import socket, AF_INET, SOCK_STREAM


def main():
    if len(sys.argv) != 2:
        print("*** ERROR usage: make client PORT=<port_num>")
        return 1

    port = int(sys.argv[1])
    sock = socket(AF_INET, SOCK_STREAM)

    if not connect(sock, port):
        return 1
    
    session_loop(sock, port)

    

def session_loop(sock, port):
    loop = 1

    while(loop):
            cmd = input("ftp> ")
            print("cmd: {}".format(cmd))
            if cmd == "-l" or cmd == "-g":
                handle_cmd(sock, cmd, port)
            elif cmd is "q":
                print("Closing connection")
                sock.close()
            else:
                print("ftp> {} command not implemented".format(cmd))



def handle_cmd(sock, cmd, port):
    
    dataPort = send_cmd(sock, cmd, port)
    
    if dataPort:
        recv_data(sock, dataPort)


def send_cmd(sock, cmd, port):
    sock.send((cmd + '\n').encode())
    res = sock.recv(3).decode()

    if res != "150":
        return 0
    else:
        dataPort = str((port-1)) + '\n'
        print("CLIENT: server status: {}".format(res))
        print("CLIENT: sending server data port {}".format(port-1))
        sock.send(dataPort.encode())
        
    return dataPort


def recv_data(cmdSock, port):
    sock = socket(AF_INET, SOCK_STREAM)
    sock.bind(('', int(port)))
    sock.listen(1)
    print("Client listening on {}".format(port))

    dataSock, addr = sock.accept()
    print("CLIENT: {} connected at {}".format(addr, port))
    sock.close()

    dataLen = cmdSock.recv(32)
    dataLen = int(dataLen.decode())
    
    print("CLIENT: Expecting data length {}".format(dataLen))
    
    cmdSock.send("220\n".encode())
    data = dataSock.recv(dataLen)
    print(data)





def connect(sock, port):
    sock.connect(('localhost', port))
    res = sock.recv(32).decode()

    if res.split()[0] != "220":
        print("CLIENT ERROR: cannot establish control connection with {} port {}\
        ".format('localhost', port))
        print("Closing program")
        sock.close()        
        return 0
    else:
        print("CLIENT: connected to server on port {}".format(port))
        return 1

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


def prompt(res):
    print("ftp> {}\n".format(res))


main()
