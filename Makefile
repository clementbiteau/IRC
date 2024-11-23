# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: soelalou <soelalou@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/12/27 09:30:42 by soelalou          #+#    #+#              #
#    Updated: 2024/02/17 16:01:01 by soelalou         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# **************************************************************************** #
# VARIABLES

NAME		= ircserv
CC			= c++
CFLAGS		= -Wall -Wextra -Werror -Iinc -std=c++98 -g3
RM			= rm -rf

SRCS_DIR	= src
OBJS_DIR	= bin

SRCS		= $(shell find $(SRCS_DIR) -type f -name '*.cpp')
OBJS		= $(patsubst src/%.cpp, $(OBJS_DIR)/%.o, $(SRCS))

# **************************************************************************** #
# COLORS

GREEN		= \033[0;32m
GREY		= \033[1;30m
PURPLE		= \033[0;35m
BLUE		= \033[0;94m
CYAN		= \033[0;36m
PINK		= \033[1;35m
RED			= \033[0;31m
END_COLOR	= \033[0;39m


# **************************************************************************** #
# RULES

all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) $(LIBFT) -o $(NAME) $(FLAGS)
	@echo "$(GREEN)[Success]$(END_COLOR) Ircserv is ready !"

$(OBJS_DIR)/%.o: src/%.cpp
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@$(RM) $(OBJS_DIR)
	@echo "$(GREY)[Clean]$(END_COLOR) Objects have been deleted"

fclean: clean
	@$(RM) $(NAME)
	@echo "$(GREY)[Clean]$(END_COLOR) $(NAME) and executables have been deleted"

re: fclean all
	@echo "$(PURPLE)[Rebuild]$(END_COLOR) Done."

.PHONY: all clean fclean re