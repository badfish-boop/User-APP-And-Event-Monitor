CC = gcc
CFLASS = -Wall -g 
CXXFLASS = 
INCLUDE = -I ./
LIBVAR  = -ldbus-1
LIBXRANDR_INC = $(shell pkg-config --cflags dbus-1)
CFLASS += $(LIBXRANDR_INC)
LIBXRANDR_LIBS = $(shell pkg-config --libs dbus-1) #$(shell pkg-config --libs x11)
LIBS += $(LIBXRANDR_LIBS)
#TARGET_LIB = liblibkysset.so
TARGET_EXE = dbus_client
#LIBPATH = ./src/liblibkysset.so

#OBJS_LIB = libkysset.o transform.o
#SRCS_LIB = ./src/libkysset.c ./src/transform.c
OBJS_EXE = dbus_client.o
SRCS_EXE = dbus_client.c

#$(OBJS_LIB):$(SRCS_LIB)
#	$(CC)  -fPIC $(CFLASS) $(LIBS) $(INCLUDE) -c $^


#	$(CC) -shared -fPIC -o $(TARGET_LIB) $(OBJS_LIB)
#	mv $(TARGET_LIB) $(LIBPATH)

$(OBJS_EXE):$(SRCS_EXE)
	$(CC)  $(CFLASS) $(INCLUDE) $(LIBS)  -c $^
	$(CC)  -o $(TARGET_EXE) $(OBJS_EXE) $(LIBVAR)


clean:
	rm -f *.o 
	rm -f  ./$(TARGET_EXE)	

.PHONY:clean
