# include "Utils.hpp"
# include "Server.hpp"

void	Server::capls(int fd) {
	sendMessage(fd, "CAP * LS :PASS NICK USER JOIN PART LIST PRIVMSG NOTICE MODE KICK INVITE TOPIC PING QUIT\r\n");
}

void	Server::pass(int fd, std::istringstream &iss, User *user) {
	std::string password;

	iss >> password;
	
	if (password == _password)
		user->setFlags("isRegistered", true);
	else
		sendErrorMessage(fd, ":localhost 464 :Password incorrect\r\n");
}

void	Server::nick(int fd, std::istringstream &iss, User *user) {
	std::string nickname;

	iss >> nickname;

	if (user->getNickname() == "") {
		int	count = getSameNicknameAmount(nickname);
		if (count > 0)
			nickname += itoa(++count);
		user->setNickname(nickname);
		sendMessage(user->getFd(), RPL_WELCOME(nickname));
		return ;
	}

	if (user->getNickname() != "" && user->getNickname() != nickname) {
		std::string oldNickname = user->getNickname();
		
		user->setNickname(nickname);
		std::string RPLMessage = ":" + oldNickname + " NICK " + nickname + "\r\n";
		sendMessage(user->getFd(), RPLMessage);
		return;
	}

	if (user != NULL && user->getNickname() == "" && user->getFd() != fd) {
		int	count = getSameNicknameAmount(nickname);
		if (count > 0)
			nickname += itoa(++count);

		user->setNickname(nickname);
		sendMessage(user->getFd(), RPL_WELCOME(nickname));
	}

}

void	Server::join(int fd, std::istringstream &iss, User *user) {
	std::string channel;
	std::string password;

	iss >> channel;
	iss >> password;

	std::cout << "ok" << std::endl;
	if (!user->getFlags().first_join_ignore) {
		user->setFlags("first_join_ignore", true);
		return ;
	}

	std::cout << "ok1" << std::endl;
	Channel *chan = getChannel(channel);
	if (chan == NULL) {
		createChannel(channel, *user);
		return;
	}

	std::cout << "ok2" << std::endl;
	if (chan->getChannelPassword() != "" && password != chan->getChannelPassword()) {
		sendErrorMessage(fd, "475 " + channel + " :Bad channel password\r\n");
		return ;
	}

	std::cout << "ok3" << std::endl;
	for (size_t i = 0; i < chan->getMembersList().size(); i++) {
		if (chan->getMembersList()[i].getNickname() == user->getNickname()) {
			sendErrorMessage(fd, "443 " + channel + " :is already a member of the channel\r\n");
			return ;
		}
	}

	std::cout << "ok4" << std::endl;
	std::string notification = "joined the channel";
	chan->sendMessageToChannel(notification, user->getNickname());
	chan->addUser(*user);
	std::string joinMessage = RPL_TOPIC(user->getNickname(), chan->getChannelName(), chan->getChannelTopic())		+ 
									RPL_NAMREPLY(user->getNickname(), chan->getChannelName(), chan->getMembersListNames())	+ 
									RPL_ENDOFNAMES(user->getNickname(), chan->getChannelName());
	sendMessage(fd, joinMessage);
	// usleep(50000);
}

void	Server::kick(int fd, std::istringstream &iss, User *user) {
	std::string moderator;
	std::string channel;

	iss >> moderator;
	iss >> channel;
	(void)user;

	Channel *chan = getChannel(channel);
	if (chan == NULL) {
		sendErrorMessage(fd, "403 " + channel + " :No such channel\r\n");
		return ;
	}

	User *userToKick = getUser(fd);
	if (userToKick == NULL) {
		sendErrorMessage(fd, "451 :You're not registered\r\n");
		return ;
	}

	(*chan).removeUser(*userToKick);

	sendMessage(fd, "441 " + moderator + " " + channel + "\r\n");
}

void   Server::invite(int fd, std::istringstream &iss, User *user) {
	std::string inviter;
	std::string channel;

	iss >> inviter;
	iss >> channel;
	(void)user;

	Channel *chan = getChannel(channel);
	if (chan == NULL) {
		sendErrorMessage(fd, "403 " + channel + " :No such channel\r\n");
		return ;
	}

	sendMessage(fd, "341 " + inviter + " " + channel + "\r\n");
}

void   Server::topic(int fd, std::istringstream &iss) {
	std::string channel;
	std::string topic;

	iss >> channel;
	iss >> topic;

	std::cout << "Channel: " << channel << " | Topic: " << topic << std::endl;

	Channel *chan = getChannel(channel);
	if (chan == NULL) {
		sendErrorMessage(fd, "403 " + channel + " :No such channel\r\n");
		return ;
	}

	chan->setChannelTopic(topic);
	sendMessage(fd, "332 " + channel + " :" + topic + "\r\n");
}

void   Server::mode(int fd, std::istringstream &iss) {
	// set the mode (i, t, k, o, and l only)

	std::string channel;
	std::string mode;
	std::string password;

	iss >> channel;
	iss >> mode;
	iss >> password;

	Channel *chan = getChannel(channel);
	if (chan == NULL) {
		sendErrorMessage(fd, "403 " + channel + " :No such channel\r\n");
		return ;
	}

	if (mode == "i" || mode == "t" || mode == "k" || mode == "o" || mode == "l") {
		if (mode == "k" && !check(atoi(password.c_str()), password)) {
			std::string client = USER(std::string("server"), std::string("server"));
			sendMessage(fd, ERR_INVALIDMODEPARAM(client, channel, mode, password));
			return ;
		}
		chan->addMode(mode);
		sendMessage(fd, "324 " + channel + " " + mode + "\r\n");
	} else {
		sendMessage(fd, "472 " + mode + " :is unknown mode char to me\r\n");
	}
}