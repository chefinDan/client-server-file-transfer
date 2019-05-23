# ==============================================================================
# Author: Daniel Green
# Date: 5 May 2019
# Description: This program initiates a tcp connection with the host created
# by compling and executing chatclient.c. It is a socket programming
# implementation of a simple two-host chat server.
# The user may enter \quit at anytime to end the chat session.
# ==============================================================================

"""  Code source citations
    The basic socket prgramming interface used in this program was taken from
    the following resource(s).

    Title: Computer Networking A Top Down Approach 7th Edition
    Author: James Kurose, Keith Ross
    Date: 2017
"""


from socket import *
import sys

MAX_MSG_SIZE = 500
MAX_HANDLE_SIZE = 10
handle = "Hal"  # The name the server will be recognized by

# =============================================================================
# This function starts a chat session with the client by first recieving the
# user's handle, and sending a response message to the client.
# Then the function enters a loop in which the client sends a message first,
# and then the server responds.


def comm(connection, addr):
    userHandle = connection.recv(MAX_HANDLE_SIZE)
    greeting = "Hello {}, you are now chatting with {}".format(
        userHandle.decode(), handle
    )

    connection.send(greeting.encode())
    while True:
        try:
            response = connection.recv(MAX_MSG_SIZE)
            # print("SERVER: recieved {}".format(message.decode()))
            if len(response.decode()) > 0:
                if response.decode() == "\\quit\n":
                    print("Client closed socket")
                    return 0

                print(userHandle.decode() + "> " + response.decode())
                del response

                # Get input to send the client
                message = input("Hal> ")
                connection.send(message.encode())
                if message == "\\quit":
                    print("Closing socket")
                    return 1

            else:
                continue

        except Exception:
            print(Exception)
            print("SERVER: cannot recieve message")


# ==============================================================================
# This is the main entry point for chatserver.py


PORT = int(sys.argv[1])
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.bind(("", PORT))

serverSocket.listen(1)

while True:
    connectionSocket, addr = serverSocket.accept()
    print("SERVER: {} connected".format(addr))
    commResult = comm(connectionSocket, addr)
    if commResult > 0:
        connectionSocket.close()
        serverSocket.close()
        break
