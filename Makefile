all:
	g++ client.cpp -o client -std=c++17 -lboost_serialization -Wno-deprecated
	g++ server.cpp -o server -std=c++17 -lboost_serialization -Wno-deprecated

