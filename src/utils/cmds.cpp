# include "Utils.hpp"
# include "Server.hpp"

void	Server::capls(int fd) {
	sendMessage(fd, "CAP * LS :PASS NICK USER JOIN PART LIST PRIVMSG NOTICE MODE KICK INVITE TOPIC PING QUIT\r\n");
}

void Server::pass(int fd, std::istringstream &iss, User *user) {
    std::string password;

    iss >> password;

    // Check if user is already authenticated
    if (user->getIsAuth()) {
        sendErrorMessage(fd, "421 PASS :Already authenticated.\r\n");
        return;
    }

    if (password == _password) {
        user->setIsAuth(true);
        std::cout << "User " << user->getFd() << " authenticated successfully." << std::endl;

        if (!user->getNickname().empty()) {
            user->setFlags("isRegistered", true);
            sendMessage(fd, GREEN "Password correct :Client Registered.\r\n" WHITE);
        } else {
            sendMessage(fd, GREEN "Password correct, awaiting NICK command to complete registration.\r\n" WHITE);
        }
    } else {
        sendErrorMessage(fd, ":localhost 464 :Password incorrect\r\n");
    }
}


void Server::nick(int fd, std::istringstream &iss, User *user) {
    std::string nickname;
    iss >> nickname;

    if (nickname.empty()) {
        sendMessage(fd, ":localhost 431 :No nickname given\r\n");
        return;
    }

    if (user->getNickname().empty()) { // First-time nickname assignment
        int count = getSameNicknameAmount(nickname);
        if (count > 0) {
            nickname += std::to_string(count + 1);
        }
        user->setNickname(nickname);

        std::cout << "fd: " << fd << " | nickname: " << nickname << std::endl;

        if (!user->getUsername().empty()) {
            user->setFlags("isRegistered", true);
            sendMessage(fd, RPL_WELCOME(nickname));
        } 
		else {
            sendMessage(fd, ":localhost 001 :Waiting for USER command to complete registration.\r\n");
        }
        return;
    }

    // Nickname change
    if (user->getNickname() != nickname) {
        int count = getSameNicknameAmount(nickname);
        if (count > 0) {
            sendMessage(fd, ":localhost 433 " + nickname + " :Nickname is already in use\r\n");
            return;
        }

        std::string oldNickname = user->getNickname();
        user->setNickname(nickname);

        std::cout << "fd: " << fd << " | nickname: " << nickname << std::endl;

        std::string RPLMessage = ":" + oldNickname + " NICK " + nickname + "\r\n";
        sendMessage(fd, RPLMessage);

        sendMessageToAllUsers(RPLMessage);
    } 
	else {
        sendMessage(fd, ":localhost 433 " + nickname + " :Nickname is unchanged\r\n");
    }
}

void Server::join(int fd, std::istringstream &iss, User *user) {
    std::string channel;
    std::string password;

    iss >> channel;
    iss >> password;

    // Log the join attempt from the user
    std::cout << "User " << user->getNickname() << " attempting to join channel " << channel << "..." << std::endl;

    Channel *chan = getChannel(channel);
    if (chan == NULL) {
        createChannel(channel, *user);
        std::cout << "User " << user->getNickname() << " created and joined channel " << channel << std::endl;
        return;
    }
    std::cout << "Channel #" << channel << " exists, checking password" << std::endl;

    if (chan->getChannelPassword() != "" && password != chan->getChannelPassword()) {
        sendErrorMessage(fd, "475 " + channel + " :Bad channel password\r\n");
        std::cout << "User: " << user->getNickname() << " failed to join channel #" << channel << " due to incorrect password" << std::endl;
        return;
    }

    // Is user a member ?
    for (size_t i = 0; i < chan->getMembersList().size(); i++) {
        if (chan->getMembersList()[i].getNickname() == user->getNickname()) {
            sendErrorMessage(fd, "443 " + channel + " :is already a member of the channel\r\n");
            std::cout << "User: " << user->getNickname() << " is already a member of channel #" << channel << std::endl;
            return;
        }
    }

    // Add user and broadcast info
    std::string notification = user->getNickname() + " has joined the channel.";
    chan->sendMessageToChannel(notification, user->getNickname());
    chan->addUser(*user);

    // Output in User -> info of channel Topic / Participants / End of Names List
    std::string joinMessage = RPL_TOPIC(user->getNickname(), chan->getChannelName(), chan->getChannelTopic()) + 
                                RPL_NAMREPLY(user->getNickname(), chan->getChannelName(), chan->getMembersListNames()) + 
                                RPL_ENDOFNAMES(user->getNickname(), chan->getChannelName());
    sendMessage(fd, joinMessage);

    std::cout << "User " << user->getNickname() << " successfully joined channel #" << channel << std::endl;
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