
ne_so=libchecksum-nautilus.so
ne4_so=libchecksum-nautilus-4.so

all:
	make $(ne_so).dir || true
	make $(ne4_so).dir || true

$(ne_so):checksum-nautilus.c
	$(CC) $(NE_FLAG) -fPIC -o $@ $< \
		-shared -Wl,-soname,$@ `pkg-config --libs --cflags libnautilus-extension`
	pkg-config  --variable=extensiondir libnautilus-extension >$(ne_so).dir
	strip --strip-unneeded $@

$(ne4_so):checksum-nautilus.c
	$(CC) -DNE_4=1 -fPIC -o $@ $< \
		-shared -Wl,-soname,$@ `pkg-config --libs --cflags libnautilus-extension-4`
	pkg-config  --variable=extensiondir libnautilus-extension-4 >$(ne4_so).dir
	strip --strip-unneeded $@

$(ne_so).dir:libchecksum-nautilus.so
$(ne4_so).dir:libchecksum-nautilus-4.so

install:
	d=$(shell cat $(ne_so).dir || true); test -n "$$d" && test -f $(ne_so) && install -d $(ROOT_DIR)$${d} && install -m 744 $(ne_so) $(ROOT_DIR)/$${d} || true
	d4=$(shell cat $(ne4_so).dir || true); test -n "$$d4" && test -f $(ne4_so) && install -d $(ROOT_DIR)$${d4} && install -m 744 $(ne4_so) $(ROOT_DIR)/$${d4} || true

clean:
	rm -rf *.so ne*.so.dir
