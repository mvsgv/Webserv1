NAME	= webserv
CXX		= c++ 
CXXFLAGS = -Wall -Werror -Wextra  -std=c++98

SRCS = 	src/main.cpp \
		src/location.cpp \
		src/ServerConfig.cpp \
		src/ConfigParser.cpp \
		src/ServerManager.cpp

OBJS =  $(SRCS:.cpp=.o)

all : $(NAME)

$(NAME) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

clean:
	rm -f $(OBJS)

fclean:
	rm -f $(NAME)

re: fclean all

.PHONY : all clean fclean re
