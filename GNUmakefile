CFLAGS+=-DNDEBUG -Os -Wall -Wextra -pedantic -std=c89

EXEC:=login
SRCS:=\
	login.c \
	template.c \
	cgi.c \
	md5.c

OBJS:=$(SRCS:%.c=%.o)

$(EXEC) : $(OBJS)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: all clean clean-all distclean realclean mostlyclean backup

all: $(EXEC)

clean:
	-$(RM) $(OBJS)
	-$(RM) depend.mk

clean-all: clean
	-$(RM) $(EXEC)
	-$(RM) *~

distclean: clean-all

backup: distclean
	-zip -r ../uamlogin-`date +%Y-%m-%d`.zip .

###
### SOURCE FILE DEPENDENCIES ###
###

depend.mk : $(SRCS)
	$(CC) $(CPPFLAGS) -MM $^ >$@

-include depend.mk
