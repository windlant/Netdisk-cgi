.PHONY:all clean
TARGETS=Windows-register.cgi Windows-login.cgi Windows-upload.cgi Windows-create-task.cgi Windows-getfiles.cgi Windows-other.cgi Windows-download.cgi Windows-get-task.cgi
CC = g++
CP = @/bin/cp
HEADDIR= ./
all:$(TARGETS)
	
	
%.cgi:%.cpp tools.o
	$(CC) -I$(HEADDIR)  `mysql_config --cflags --libs` $^ -o $@


tools.o:tools.cpp
	$(CC) -I$(HEADDIR)  -c $^ -o $@

clean:
	@rm -rf $(TARGETS) tools.o

