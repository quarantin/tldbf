TARGET=tldbf
SOURCE=$(TARGET).c
TLDFILE=tlds-alpha-by-domain.txt
TLDURL=http://data.iana.org/TLD/$(TLDFILE)
TLDFILEDEST=/usr/share/tldbf/

all:
	gcc -o $(TARGET) $(SOURCE)

update:
	wget -O $(TLDFILE) $(TLDURL)

clean:
	rm -f $(TARGET)

install:
	cp $(TARGET) /usr/local/bin/
	chmod 755 /usr/local/bin/$(TARGET)
	mkdir -p $(TLDFILEDEST)
	cp $(TLDFILE) $(TLDFILEDEST)

uninstall:
	rm -f /usr/local/bin/$(TARGET)
	rm -f $(TLDFILEDEST)/$(TLDFILE)
	rmdir $(TLDFILEDEST)
