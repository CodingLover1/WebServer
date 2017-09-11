WebServer:main.o http_conn.o
	g++ $^ -o $@ -lpthread

main.o:main.cpp locker.h threadpool.h http_conn.h
	g++ -c $^

http_conn.o: http_conn.cpp http_conn.h
	g++ -c $^
	
clean:
	rm -f *.o

