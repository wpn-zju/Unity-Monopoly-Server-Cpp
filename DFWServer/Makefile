# compile macro  
CC      = g++  
CFLAGS  = -Wall  
LDFLAGS = -lm -lpthread
INCLUDES= -I/usr/local/include
  
# user modify: target and objects  
EXENAME = Main
OBJECTS = Main.o enet.o CardManager.o Event.o GameSettings.o MapHelper.o MapPoint.o Player.o Utility.o ENetController.o Room.o GameInstance.o json.o
  
# make target  
all:${OBJECTS}  
	${CC} -o ${EXENAME} *.o ${LDFLAGS} ${INCLUDES}  
      
# make clean  
clean:  
	rm -rf ${EXENAME} ${OBJECTS}   

# dependence      
%.o : %.cpp  
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@  
%.o : %.cc  
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@  