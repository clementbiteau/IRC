#ifndef DEFINES_HPP
#define DEFINES_HPP

#include <iostream>
#include <vector>
#include <sys/socket.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <csignal>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sstream>

struct flags {
	bool		first_join_ignore;
	bool		isRegistered;
};

#define WHITE "\033[1;37m"
#define RED "\033[1;31m"
#define GREEN "\e[1;32m"
#define ORANGE "\e[1;33m"
#define BLUE "\e[1;34m"

#define USER(nickname, username) (":" + nickname + "!" + username + "@localhost")
#define RPL_JOIN(user_id, channel) (user_id + " JOIN :" + channel + "\r\n")
#define RPL_INVITE(user_id, invited, channel) (user_id + " invited " + invited + " in #" + channel + "\r\n")
#define MSG(color, prefix, msg) (std::cout << color << "[" << prefix << "] " << WHITE << msg << std::endl)
#define CLIENT_MSG(color, prefix, msg, nb, space, msg2) (std::cout << color << "[" << prefix << "] " << WHITE << msg << nb << space << msg2 << std::endl)
#define ERR(msg) (std::cout << RED << "[ERROR] " << WHITE << msg << std::endl)
#define ERR_INVALIDMODEPARAM(client, channel, mode, password) ("696 " + client + " #" + channel + " " + mode + " " + password + " : password must only contained alphabetic character\r\n")

#define RPL_WELCOME(clientNickname)                                (std::string(":localhost 001 ") + clientNickname + " :Welcome to the server !~" + clientNickname + "@localhost! \r\n")
#define RPL_PART(clientNickname, channel)							(std::string(":localhost 324 ") + clientNickname + " " + channel + " :You have left " + channel + ".\r\n")
#define RPL_PONG(param)					                        ("PONG :" + std::string(param) + ".\r\n")
#define RPL_PARTNOTICE(clientNickname, channel)					(":localhost 400 " + clientNickname + " " + channel + " :- You have left the channel.\r\n")


#define RPL_CHANNELMODEIS(channel, modes)                          (":localhost 324 " + channel + " " + modes + ".\r\n")
#define RPL_TOPIC(clientNickname, channel, topic)		            (":localhost 332 " + clientNickname + " " + channel + " :" + topic + ".\r\n")
#define RPL_NOTOPIC(clientNickname, channel)                       (":localhost 331 " + clientNickname + " " + channel + " :No topic is set.\r\n")
#define RPL_INVITING(clientNickname, invitedNick, channel)         (":localhost 341 " + clientNickname + " " + invitedNick + " " + channel + " :Was succesfully invited.\r\n")    
#define RPL_NAMREPLY(clientNickname, channel, names)	            (":localhost 353 " + clientNickname + " = " + channel + " :" + names +".\r\n")
#define RPL_ENDOFNAMES(clientNickname, channel)		            (":localhost 366 " + clientNickname + " " + channel + " :End of /NAMES list.\r\n")
#define RPL_ALREADYREGISTRED(clientNickname, channel)	            (":localhost 403 " + clientNickname + " " + channel + " :You are already in that channel.\r\n")

#endif