import random
import string


def randMessage(length):

    charArr = []
    for i in range(length):
        charArr.append(random.choice(string.ascii_letters))

    return ''.join(charArr)
