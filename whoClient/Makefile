objects = src/main.o src/utils.o

whoClient : $(objects)
		cc -o whoClient $(objects) -lpthread
		@echo "========================================================================"
		@echo "|Try running: ./whoClient -q queries.txt -w 10 -sp 156 -sip 127.0.0.1         |"
		@echo "|														              |"
		@echo "========================================================================"

src/main.o : include/Interface.h
src/utils.o : include/Interface.h

.PHONY: clean
clean :
		@echo "Cleaning ..."
		rm whoClient $(objects)
