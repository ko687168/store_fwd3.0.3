# Makefile for systems with GNU tools
#CC 	=	gcc
INSTALL	=	install
IFLAGS  = -idirafter dummyinc
#CFLAGS = -g
CFLAGS = -fno-pie      -O2 -fstack-protector --param=ssp-buffer-size=4 \
	-W -Wshadow  \
	-D_FORTIFY_SOURCE=2 \
	#-pedantic -Wconversion

# LIBS	=	`./vsf_findlibs.sh`
LINK	=   -s
LDFLAGS        =  -ldl -lssl -lcrypto

OBJS	=	main.o utility.o prelogin.o ftpcmdio.o postlogin.o privsock.o \
		tunables.o ftpdataio.o secbuf.o ls.o \
		postprivparent.o logging.o str.o netstr.o sysstr.o strlist.o \
    banner.o filestr.o parseconf.o secutil.o \
    ascii.o oneprocess.o twoprocess.o privops.o standalone.o hash.o \
    tcpwrap.o ipaddrparse.o access.o features.o readwrite.o opts.o \
    ssl.o sslslave.o ptracesandbox.o ftppolicy.o sysutil.o sysdeputil.o \
    seccompsandbox.o cJSON.o auth_user.o


.c.o:
	$(CC) -c $*.c $(CFLAGS) $(IFLAGS)

store_fwd: $(OBJS) 
	$(CC) -o store_fwd $(OBJS) $(LINK) $(LDFLAGS)

install:
	if [ -x /usr/local/sbin ]; then \
		$(INSTALL) -m 755 vsftpd /usr/local/sbin/store_fwd; \
	else \
		$(INSTALL) -m 755 vsftpd /usr/sbin/store_fwd; fi
	if [ -x /etc/xinetd.d ]; then \
		$(INSTALL) -m 644 xinetd.d/vsftpd /etc/xinetd.d/vsftpd; fi

clean:
	rm -f *.o *.swp store_fwd

