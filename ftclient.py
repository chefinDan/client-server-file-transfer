from socket import socket, AF_INET, SOCK_STREAM
import random
import string


def randMessage(length):
    charArr = []
    for i in range(length):
        charArr.append(random.choice(string.ascii_letters))

    return ''.join(charArr)


messageSize = 32
serverName = 'localhost'
serverPort = 10021
clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.connect((serverName, serverPort))

# message = randMessage(messageSize)
message = "Hello"

clientSocket.send(message.encode())
serverResponse = clientSocket.recv(messageSize)
response = serverResponse.decode()
print('ftp> ', response)
clientSocket.close()
