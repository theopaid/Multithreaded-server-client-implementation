objects = src/main.o src/helpfulFunctions.o src/menuFunctions.o src/handler.o src/recordList.o src/hashTable.o src/avlTree.o src/freeFunct.o

diseaseAggregator : $(objects)
		cc -o diseaseAggregator $(objects)
		@echo "========================================================================"
		@echo "|Try running: ./diseaseAggregator  -w 3 -b 250 -i input_dir            |"
		@echo "|														              |"
		@echo "========================================================================"

src/main.o : include/Interface.h
src/helpfulFunctions.o : include/Interface.h
src/menuFunctions.o : include/Interface.h
src/handler.o : include/Interface.h
src/recordList.o : include/Interface.h
src/hashTable.o : include/Interface.h
src/avlTree.o : include/Interface.h
src/freeFunct.o : include/Interface.h

.PHONY: clean
clean :
		@echo "Cleaning ..."
		rm diseaseAggregator $(objects)
