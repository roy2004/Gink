PREFIX = /usr/local/
OBJECTS = Coroutine.o\
          GAIError.o\
          Stream.o\
          SystemError.o\
          TCPSocket.o
CPPFLAGS = -iquote Include -MMD -MT $@ -MF Build/$*.d
CPPFLAGS += -DNDEBUG
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror
CXXFLAGS += -O2
ARFLAGS = rc

all: Build/Library.a

Build/Library.a: $(addprefix Build/, $(OBJECTS))
	$(AR) $(ARFLAGS) $@ $^

ifneq ($(MAKECMDGOALS), clean)
-include $(patsubst %.o, Build/%.d, $(OBJECTS))
endif

Build/%.o: Source/%.cxx
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f Build/*

install: all
	mkdir -p $(PREFIX)lib
	cp -T Build/Library.a $(PREFIX)lib/libgink.a
	mkdir -p $(PREFIX)include
	cp -r -T Include $(PREFIX)include/Gink

uninstall:
	rm $(PREFIX)lib/libgink.a
	rmdir -p --ignore-fail-on-non-empty $(PREFIX)lib
	rm -r $(PREFIX)include/Gink
	rmdir -p --ignore-fail-on-non-empty $(PREFIX)include
