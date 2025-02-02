all: serverM.cpp serverA.cpp serverB.cpp client.cpp

	g++ -std=c++17 -o serverM serverM.cpp

	g++ -std=c++17 -o serverA serverA.cpp

	g++ -std=c++17 -o serverB serverB.cpp

	g++ -std=c++17 -o client client.cpp

clean:
	rm -rf *.o client serverA serverB serverM
	
