CXXFLAGS = -W -Wall -Wextra -std=c++11
LDFLAGS = -lboost_system -lpthread

SOURCES = main.cpp \
		  server.cpp \
		  connection.cpp \
		  request.cpp \
		  log.cpp \
		  error.cpp

.PHONY: all clean

all: server

server: main.o server.o connection.o request.o log.o error.o
	$(CXX) $^ $(LDFLAGS) -o $@

clean:
	rm *.o *.d server

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(subst .cpp,.d,$(SOURCES))
endif
endif

%.d: %.cpp
	@$(CXX) -MM $(CXXFLAGS) $< > $@
