SRC_DIR		:= src
CC			:= gcc
AR			:= ar

DEPFLAGS	= -MD -MP
CFLAGS		= -std=gnu11 -g -O3 -march=native $(DEPFLAGS) -Wall -Werror -Wextra
ARFLAGS		= -rsc

SRCS		= $(wildcard $(SRC_DIR)/*.c)
OBJS		= $(SRCS:$(SRC_DIR)/%.c=%.o)
VPATH		= $(SRC_DIR)

LIB			= mtalloc.a

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $<

clean:
	$(RM) $(LIB) *.o *.d

.PHONY: all clean
