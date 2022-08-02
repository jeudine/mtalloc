SRC_DIR		:= src
LIB_DIR		:= ../src
CC			:= gcc

DEPFLAGS	= -MD -MP
CFLAGS		= -std=gnu11 -g -O3 $(DEPFLAGS) -Wall -Werror
LDLIBS		= -pthread

SRCS		= $(wildcard $(SRC_DIR)/*.c)
LIB_SRCS		= $(wildcard $(LIB_DIR)/*.c)
OBJS		= $(SRCS:$(SRC_DIR)/%.c=%.o)
LIB_OBJS		= $(LIB_SRCS:$(LIB_DIR)/%.c=%.o)
VPATH		= $(SRC_DIR) $(LIB_DIR)

EXE			= mtalloc_test

all: $(EXE)

$(EXE) : $(OBJS) $(LIB_OBJS)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

run: $(EXE)
	./$<

clean:
	$(RM) $(EXE) *.o *.d

.PHONY: all clean run
