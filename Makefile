CC = g++

all: erf2pcap

erf2pcap: erf2pcap.cpp
	$(CC) -l trace -o erf2pcap erf2pcap.cpp

clean:
	rm -f erf2pcap
