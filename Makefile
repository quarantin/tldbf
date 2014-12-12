TARGET=tldbf
SOURCE=$(TARGET).c

all:
	gcc -o $(TARGET) $(SOURCE)

update:
	wget -O tlds-alpha-by-domain.txt http://data.iana.org/TLD/tlds-alpha-by-domain.txt

clean:
	rm -f $(TARGET)
