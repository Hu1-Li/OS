CC=gcc
OBJS=1-1 1-2
all: $(OBJS) 
$(OBJS): % : %.c 
	$(CC) $< -o $@ 
.PHONY: clean
clean:
	rm -f $(OBJS) *.o *~


