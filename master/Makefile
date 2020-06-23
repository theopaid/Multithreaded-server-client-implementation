objects = src/main.o src/helpfulFunctions.o src/handler.o src/recordList.o src/hashTable.o src/avlTree.o src/freeFunct.o src/StatsLists.o

master : $(objects)
		cc -o master $(objects)
		@echo "========================================================================"
		@echo "|Try running: ./master -w 3 -b 4096 -s 127.0.0.1 -p 1166 -i input_dir  |"
		@echo "|														              |"
		@echo "========================================================================"

src/main.o : include/Interface.h
src/helpfulFunctions.o : include/Interface.h
src/handler.o : include/Interface.h
src/recordList.o : include/Interface.h
src/hashTable.o : include/Interface.h
src/avlTree.o : include/Interface.h
src/freeFunct.o : include/Interface.h
src/StatsLists.o : include/Interface.h

.PHONY: clean
clean :
		@echo "Cleaning ..."
		rm master $(objects)
