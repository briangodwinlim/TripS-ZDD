OPT = -O3 -DB_64 -I. -ISAPPOROBDD/include -ITdZdd/include -Ijson/include -fopenmp

all: trips-zdd

trips-zdd: trips-zdd.cpp SAPPOROBDD/lib/BDD64.a GeoDistributedStorageSystem.hpp ValidConfig.hpp GetConfig.hpp WeightedIterator.hpp
	g++ trips-zdd.cpp SAPPOROBDD/lib/BDD64.a -o trips-zdd $(OPT)

clean:
	rm -f trips-zdd trips-zdd.exe *.o
