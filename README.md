# FileTransfer
Very Simple Network File Transfer

## Tools & Dependencies
* gcc 9.1
* GNU Make 4.2.1 
* boost 1.67.0
* boost::asio 1.67.0
* Google Test 1.8.1 

## Build
´make´

## Run as server
´./FileTransferTest -s -p 12000 -a 0.0.0.0 -f /copied_files/´

## Run as client
´./FileTransferTest -c -p 12000 -a 0.0.0.0 -f /myfile.txt´

## Run unit tests
´./FileTransferTest´