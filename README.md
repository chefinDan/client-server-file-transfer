# VS-FTP (Very Simple File Transfer Protocol)
Daniel Green, greendan@oregonstate.edu

## *** Notes to grader ***
#### This program can transfer any type of file, including binary files, so I believe this is worth extra credit points.



## How to use

1. After cloning this repository or unpacking the zip, cd into the top-level directory, and run the following command.

    ```Bash
    make server PORT=<listen_port>
    ```
    For example, if you are on flip1:
    
    ```Bash
        [user@flip1]$ make server PORT=12345
        ...

        ./ftserver 12345
        Server listening on localhost at port 12345
    ``` 
    Where <listen_port> will be the port that the client will connect on for the initial control connection.
    

2. Next either on the same host, or a different one such as flip2, run the following command
    
    ```Bash
        make server HOST=<server_host> CPORT=<ctrl_port> DPORT=<data_port>
    ```
    \
    For example, if the server is on flip1 listening on port 12345, and you are now on flip2:
    ```Bash

        [user@flip2]$ make client HOST=flip1 CPORT=12345 DPORT=12344
        python3 src/ftclient.py flip2 12345 12346
        ==============================================================
            Welcome to VS-FTP (Very Simple File Transfer Protocol)
                    Daniel Green, greendan@oregonstate.edu
        ==============================================================


            *** Connected to flip2.engr.oregonstate.edu on port 12345 ***

        Commands:
            -g <file_name>    download file if available
            -l                list the contents of remote directry
            -q                quit the client program

        ftp>_

    ```


