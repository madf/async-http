CXXFLAGS = -W -Wall -Wextra -std=c++11
LDFLAGS = -lboost_system -lpthread

SOURCES = main.cpp \
		  tcpserver.cpp \
		  tcpconnection.cpp \
		  request.cpp

.PHONY: all clean

all: main

main: main.o tcpserver.o tcpconnection.o request.o
	$(CXX) $^ $(LDFLAGS) -o $@

clean:
	rm *.o *.d main

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(subst .cpp,.d,$(SOURCES))
endif
endif

%.d: %.cpp
	@$(CXX) -MM $(CXXFLAGS) $< > $@
