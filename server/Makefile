OUTPUT = server_app
ZDROJAKY = src/pos_sockets/*.cpp src/*.cpp main.cpp

server: $(OUTPUT)

clean:
	rm -f $(OUTPUT)

.PHONY: server clean

$(OUTPUT):$(ZDROJAKY)
	g++ -std=c++17 -pthread $^ -o $@
