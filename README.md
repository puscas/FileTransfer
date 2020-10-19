# FileTransfer
Very Simple Network File Transfer

## Tools & Dependencies
* gcc 9.1
* GNU Make 4.2.1 
* boost 1.67.0
* boost::asio 1.67.0
* Google Test 1.8.1 

## Build
`make`

## Run as server
`./FileTransferTest -s -p 12000 -a 0.0.0.0 -f /copied_files/`
* `-s` - Run in server mode.
* `-p` - Port to handle requests.
* `-a` - Binding address.
* `-f` - Folder to store the received files.

## Run as client
`./FileTransferTest -c -p 12000 -a 0.0.0.0 -f /myfile.txt`
* `-c` - Run in client mode.
* `-p` - Connection port.
* `-a` - Connection address.
* `-f` - File to copy.

## Run unit tests
`./FileTransferTest`
