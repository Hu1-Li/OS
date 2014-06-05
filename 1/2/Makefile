CC=gcc
OBJS=get copy put
all: $(OBJS) 
$(OBJS): % : %.c 
	$(CC) $< -o $@ `pkg-config --cflags --libs gtk+-2.0`
.PHONY: clean
clean:
	rm -f $(OBJS) *.o *~


