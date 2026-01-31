ASIO=asio-asio-1-12-2

server: server.hpp main.cpp
	g++ -std=c++14 -DASIO_STANDALONE -I${ASIO}/asio/include -pthread main.cpp -o server.exe -lws2_32 -lmswsock

clean:
	powershell -Command "if (Test-Path server.exe) { Remove-Item server.exe }"

