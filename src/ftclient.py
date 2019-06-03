# /******************************************************************************
# **          VS-FTP (Very Simple File Transfer Protocol)
#                           Client software
# ** Author: Daniel Green, greendan@oregonstate.edu
# ** Last Modified: 2 June 2019
# ** Description: starts a ftp client that accepts list and get commands.
# *** Usage requires the ftserver program to be running on either localhost
# *** or a remote location. The client program will can ask for any file
# *** found in the local directory of the server prgram.
# *** The client program will exit after each executed command.
# *** The server will remain listening for connections until SIGINT is recieved.
# *** 


# ** DEMO: To demonstrate the program, first start ftserver, then start ftclient.
# ** Transfer big.txt, a 6.2 MB file, and compare downloaded byte size to client
# ** reported byte count. Then transfer cvector, a binary file, give it execute
# ** permissions and execute. It is simple program that allocates a large array
# ** on the heap and displays the size of the array. 
# ******************************************************************************/

# /*
# sources: https://linux.die.net/man/,
#          https://docs.python.org/3.7/library/socket.html
#          https://beej.us/guide/bgnet/
# */


import sys
import os
from socket import socket, AF_INET, SOCK_STREAM, SOL_SOCKET, SO_REUSEADDR

# default location for downloaded files
download_dir = 'downloads'
BUFF_SIZE = 4096 # 4 KiB

# Main entry point for client program
def main():
    # validate arguments
    if len(sys.argv) != 4:
        print("*** ERROR usage: make client HOST=<host_name> CPORT=<ctrl_port> DPORT=<data_port>")
        print("*** Host maybe be flip1-4, or localhost")
        return 1

    # assign correct hostname, port, and dataPort based on cli args
    host = sys.argv[1]
    if host == 'flip1' or host == 'flip2' or host == 'flip3' or host == 'flip4':
        host = host + '.engr.oregonstate.edu'
    
    port = int(sys.argv[2])
    dataPort = int(sys.argv[3])
    sock = socket(AF_INET, SOCK_STREAM)

    # attempt connection to the listening server
    if not connect(sock, host, port):
        return 1

    # enter the main session loop
    session_loop(sock, port, dataPort)

    print("Closing connection")
    sock.close()


# This function maintains a loop that will execute until a valid command
# is successfully entered and executed.
def session_loop(sock, port, dataPort):
    loop = 1

    while(loop):
            cmd = input("ftp> ")
            cmdlist = cmd.split()
            if cmdlist[0] == "-l" or (cmdlist[0] == "-g" and len(cmdlist) == 2):
                handle_cmd(sock, cmdlist, port, dataPort)
                loop = 0
            elif cmdlist[0] == "-g" and len(cmdlist) == 1:
                print("usage: -g <file_name>")
            elif cmdlist[0] == "-q":
                loop = 0
            else:
                print("ftp> {} command not implemented".format(cmd))


# This function sends a command to the server and if the send was successful,
# calls the recv_data function. 
def handle_cmd(sock, cmd, port, dataPort):
    if send_cmd(sock, cmd, port, dataPort):
        recv_data(sock, cmd, dataPort)
        
        

def send_cmd(sock, cmd, port, dataPort):
    
    # send cmd to server
    sock.send((cmd[0] + '\n').encode())    
    res = sock.recv(3).decode()

    
    # check for "pending further information" response  
    # print("CLIENT: server status: {}".format(res))
    if res != "350":
        return 0

    # send server data port for data transfer 
    # print("CLIENT: sending server data port {}".format(port-1))
    sock.send((str(dataPort) + '\n').encode())

    res = sock.recv(3).decode()
    # print("CLIENT: server status: {}".format(res))
    # check for "pending further information" response
    if res != "350":
        return 0
    
    # return success
    return 1



def recv_data(cmdSock, cmd, port):
    sock = socket(AF_INET, SOCK_STREAM)
    sock.setsockopt( SOL_SOCKET, SO_REUSEADDR, 1 )
    sock.bind(('', int(port)))
    sock.listen(1)

    # After turning on socket, send server 220, indicating you are listening
    print("Client listening on {}".format(port))
    cmdSock.send("220\n".encode())

    dataSock, addr = sock.accept()
    print("CLIENT: {} connected at {}".format(addr, port))
    sock.close()

    # if command is -l, then get datalength from server, send OK, recieve data, and close
    if cmd[0] == '-l':
        dataLen = cmdSock.recv(32)
        dataLen = int(dataLen.decode())
    
        # print("CLIENT: Expecting data length {}".format(dataLen))
    
        cmdSock.send("220\n".encode())
        data = dataSock.recv(dataLen)
        dataSock.close()
        print("\n")
        print(data.decode())
    else:
        # get res from server for -g cmd
        res = cmdSock.recv(3).decode()
        
        # res should be "pending further action" 350
        if res != "350":
            return 0
        
        # Now send the server the reqested file
        reqfile = str(cmd[1]).strip(' ') + '\n'
        print("client wants:",reqfile)
        cmdSock.send(reqfile.encode())
        res = cmdSock.recv(3).decode()

        # res should be 250 FILE ACTION OK
        if res == "550":
            print("550 requested action not taken, file not found")
            return 0
        elif res == "451":
            print("451 requested action aborted, server error")
            return 0
        elif res == "450":
            print("450 requested action not taken, is directory")
            return 0
        elif res == "250":
            print("250 requested file action ok.")
            cmdSock.send("220\n".encode())
            
            # recieve expected data size
            dataLen = cmdSock.recv(32)
            dataLen = int(dataLen.decode())
            print("CLIENT: Expecting data length {}".format(dataLen))
            cmdSock.send("220\n".encode())

            # proceed with recieving data from server
            data = b''
            while True:
                part = dataSock.recv(BUFF_SIZE)
                data += part
                if len(data) == dataLen:
                    #print(data)
					# eitiher 0 or end of data
                    break

            print("{} transfer complete".format(reqfile))
            dataSock.close()

            if not os.path.exists(download_dir):
                os.mkdir(download_dir)
                print("Directory " , download_dir ,  " Created ")
            # else:    
                # print("Directory " , download_dir ,  " already exists")
 
            with open(download_dir + '/' + reqfile.strip(), 'wb+') as f:
                f.write(data) 

    



def connect(sock, host, port):
    sock.connect((host, port))
    res = sock.recv(32).decode()

    if res.split()[0] != "220":
        print("CLIENT ERROR: cannot establish control connection with {} port {}\
        ".format('localhost', port))
        print("Closing program")
        sock.close()        
        return 0
    else:
        print("\n\n==============================================================")
        print("    Welcome to VS-FTP (Very Simple File Transfer Protocol)    ")
        print("           Daniel Green, greendan@oregonstate.edu             ")
        print("==============================================================\n\n")
        print("        *** Connected to {} on port {} ***              \n".format(host, port))
        print(" Commands:\n    -g <file_name>    download file if available")
        print("    -l                list the contents of remote directry")
        print("    -q                quit the client program\n")
        return 1


def prompt(res):
    print("ftp> {}\n".format(res))


main()
