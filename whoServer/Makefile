objects = src/main.o src/utils.o src/buffer.o src/workersInfoList.o

whoServer : $(objects)
		cc -o whoServer $(objects) -lpthread
		@echo "========================================================================"
		@echo "|Try running: ./whoServer -q 1156 -s 1166 -w 7 -b 10                   |"
		@echo "|														              |"
		@echo "========================================================================"

src/main.o : include/Interface.h
src/utils.o : include/Interface.h
src/buffer.o : include/Interface.h
src/workersInfoList.o : include/Interface.h

.PHONY: clean
clean :
		@echo "Cleaning ..."
		rm whoServer $(objects)
