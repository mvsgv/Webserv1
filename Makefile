NAME    = webserv
CXX     = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98

SRCS =  src/main.cpp \
        src/location.cpp \
        src/ServerConfig.cpp \
        src/ConfigParser.cpp \
        src/ServerManager.cpp \
        src/Client.cpp

OBJS =  $(SRCS:src/%.cpp=obj/%.o)

all : $(NAME)

$(NAME) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

obj/%.o: src/%.cpp
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf obj

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY : all clean fclean re
