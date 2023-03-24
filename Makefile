Parser=parser
DeBug=debug
Http_server=http_server

.PHONY:all
all:$(Parser) $(DeBug) $(Http_server)

$(Parser):parser.cpp
	g++ -o $@ $^ -lboost_system -lboost_filesystem -std=c++11 
$(DeBug):debug.cpp
	g++ -o $@ $^ -std=c++11 -ljsoncpp
$(Http_server):http_server.cpp
	g++ -o $@ $^ -std=c++11 -ljsoncpp -lpthread

.PHONY:clean
clean:
	rm -f $(Parser) $(DeBug) $(Http_server)
