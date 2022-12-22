SRC_DIR		:= src
CC			:= gcc

DEPFLAGS	= -MD -MP
CFLAGS		= -std=gnu11 -g -O3 -march=native -I../src $(DEPFLAGS) -Wall -Werror -Wextra
LDLIBS		= -pthread ../mtalloc.a

SRCS		= $(wildcard $(SRC_DIR)/*.c)
OBJS		= $(SRCS:$(SRC_DIR)/%.c=%.o)
VPATH		= $(SRC_DIR)

EXE			= mtalloc_test

all: $(EXE)

$(EXE) : $(OBJS)
	$(MAKE) -C ..
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

run: $(EXE)
	./$<

clean:
	$(RM) $(EXE) *.o *.d

.PHONY: all clean run
-include $(wildcard *.d)
