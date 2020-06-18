objects = src/main.o src/utils.o src/buffer.o

whoServer : $(objects)
		cc -o whoServer $(objects) -lpthread
		@echo "========================================================================"
		@echo "|Try running: ./whoServer -q 156 -s 166 -w 7 -b 10          |"
		@echo "|														              |"
		@echo "========================================================================"

src/main.o : include/Interface.h
src/utils.o : include/Interface.h
src/buffer.o : include/Interface.h

.PHONY: clean
clean :
		@echo "Cleaning ..."
		rm whoServer $(objects)
