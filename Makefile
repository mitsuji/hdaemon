all:
	ghc -c HDaemon/Server.hs
	ghc -c HDaemon/Daemon.hs -XForeignFunctionInterface
	ghc -c posix.c
	ghc -o hdaemond \
		-threaded \
		posix.o \
		HDaemon/Daemon_stub.o \
		HDaemon/Daemon.o \
		HDaemon/Server.o

clean:
	rm -f hdaemond
	rm -f *.o
	rm -f HDaemon/*.o
	rm -f HDaemon/*.hi
	rm -f HDaemon/*_stub.h
	rm -f HDaemon/*_stub.c

