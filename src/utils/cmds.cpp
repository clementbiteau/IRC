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

    // If the channel does not exist, create it and add the user
    if (chan == NULL) {
        createChannel(channel, *user); // Create the channel
        chan = getChannel(channel);   // Retrieve the newly created channel
        if (chan) {
            chan->addUser(*user);      // Add the user to the channel's members list
            chan->addOperator(*user);  // Automatically make the user an operator

            // Notify the user about creating and joining the channel
            std::cout << "User " << user->getNickname() << " created and joined channel " << channel << " as an operator." << std::endl;

            // Optionally, send a notification to the channel about the new operator
            std::string notification = user->getNickname() + " has created and joined the channel as an operator.";
            chan->sendMessageToChannel(notification, user->getNickname());
        } else {
            std::cerr << "Error: Failed to create channel " << channel << "." << std::endl;
            sendErrorMessage(fd, "403 " + channel + " :Failed to create channel\r\n");
        }
        return;
    }

    // Log that the channel exists
    std::cout << "Channel " << channel << " exists, checking password" << std::endl;

    // Check password if the channel is password-protected
    if (chan->getChannelPassword() != "" && password != chan->getChannelPassword()) {
        sendErrorMessage(fd, "475 " + channel + " :Bad channel password\r\n");
        std::cout << "User: " << user->getNickname() << " failed to join channel #" << channel << " due to incorrect password" << std::endl;
        return;
    }

    // Check if the user is already a member of the channel
    if (chan->isUserInChannel(*user)) {
        sendErrorMessage(fd, "443 " + channel + " :is already a member of the channel\r\n");
        std::cout << "User: " << user->getNickname() << " is already a member of channel #" << channel << std::endl;
        return;
    }

    // Add user to the channel and notify others
    chan->addUser(*user);
    std::string notification = user->getNickname() + " has joined the channel.";
    chan->sendMessageToChannel(notification, user->getNickname());

    // Send topic and names list to the user
    std::string joinMessage = RPL_TOPIC(user->getNickname(), chan->getChannelName(), chan->getChannelTopic()) + 
                               RPL_NAMREPLY(user->getNickname(), chan->getChannelName(), chan->getMembersListNames()) + 
                               RPL_ENDOFNAMES(user->getNickname(), chan->getChannelName());
    sendMessage(fd, joinMessage);

    // Log the successful join
    std::cout << "User " << user->getNickname() << " successfully joined channel " << channel << std::endl;
}

void Server::kick(int fd, std::istringstream &iss, User *user) {
    std::string moderator;
    std::string channel;
    std::string userToKickNick;

    iss >> moderator;
    iss >> channel;
    iss >> userToKickNick;

    Channel *chan = getChannel(channel);
    if (chan == nullptr) {
        sendErrorMessage(fd, "403 " + channel + " :No such channel\r\n");
        return;
    }

    // Find the user to kick by nickname
    User *userToKick = chan->getUserByNickname(userToKickNick);
    if (userToKick == nullptr) {
        sendErrorMessage(fd, "441 " + moderator + " " + channel + " " + userToKickNick + " :No such nick/channel\r\n");
        return;
    }

    // Check if the moderator is in the channel
    if (!chan->isUserInChannel(*user)) {
        sendErrorMessage(fd, "442 " + channel + " :You're not on that channel\r\n");
        return;
    }

    // Check if the user issuing the KICK is an operator
    if (!chan->isOperator(*user)) {
        sendErrorMessage(fd, "482 " + channel + " :You're not a channel operator\r\n");
        return;
    }

    // Notify the kicked user
  	sendMessage(userToKick->getFd(), "Channel: " + channel + " -> Dear " + userToKickNick + ", You have been kicked from the channel.\r\n");
    // Remove the user from the channel
    chan->removeUser(*userToKick);

    // Notify the channel about the kick
    std::string kickMessage = ":" + moderator + " Kicked out " + userToKickNick  + " from " + channel + "\r\n";
    chan->sendMessageToChannel(kickMessage, moderator);  

    // Notify the moderator
    sendMessage(fd, ":" + moderator + " successfully kicked " + userToKickNick + " from " + channel + ".\r\n");

    // Debugging output
    std::cout << "KICK command processed: Moderator: " << moderator << ", Channel: " << channel
              << ", Kicked User: " << userToKickNick << std::endl;
}

void Server::invite(int fd, std::istringstream &iss, User *user) {
    std::string inviteeNickname;
    std::string channelName;

    iss >> inviteeNickname;
    iss >> channelName;

    // Validate input
    if (inviteeNickname.empty() || channelName.empty()) {
        sendErrorMessage(fd, "461 INVITE :Not enough parameters\r\n");
        return;
    }

    Channel *chan = getChannel(channelName);
    if (chan == NULL) {
        sendErrorMessage(fd, "403 " + channelName + " :No such channel\r\n");
        return;
    }

    // Check if the inviter is an operator of the channel
    if (!chan->isOperator(*user)) {
        sendErrorMessage(fd, "482 " + channelName + " :You're not a channel operator\r\n");
        return;
    }

    // Check if the invitee is registered on the server (not just in the channel)
    User *invitee = getUserByNickname(inviteeNickname);
    if (invitee == NULL) {
        sendErrorMessage(fd, "401 " + inviteeNickname + " :No such nick\r\n");
        return;
    }

    // Check if the invitee is already in the channel
    if (chan->isUserInChannel(*invitee)) {
        sendErrorMessage(fd, "443 " + inviteeNickname + " " + channelName + " :is already on channel\r\n");
        return;
    }

    // Notify the invitee of the invite
    sendMessage(invitee->getFd(), ":" + user->getNickname() + " has invited you dear " + inviteeNickname + " to join the channel -> " + channelName + "\r\n");

    // Confirm to the inviter that the invite was sent
    sendMessage(fd, "You: " + user->getNickname() + " have invited " + inviteeNickname + " to " + channelName + "\r\n");

    std::cout << "User " << user->getNickname() << " invited " << inviteeNickname << " to channel " << channelName << std::endl;
}

void	Server::topic(int fd, std::istringstream &iss, User *user) {
    std::string channel;
    std::string topic;

    iss >> channel;

    // Is there a topic set ?
    std::getline(iss, topic);

    std::cout << "Channel: " << channel << " | Topic: " << topic << std::endl;

    Channel *chan = getChannel(channel);
    if (chan == NULL) {
        sendErrorMessage(fd, "403 " + channel + " :No such channel\r\n");
        return;
    }
    if (topic.empty()) {
        std::string currentTopic = chan->getChannelTopic();
        sendMessage(fd, "332 " + channel + " :" + (currentTopic.empty() ? "No topic set" : currentTopic) + "\r\n");
        return;
    }

    // Check if the user is an operator in the channel
    if (!chan->isOperator(*user)) {
        sendErrorMessage(fd, "482 " + channel + " :You're not a channel operator\r\n");
        return;
    }

    // Set the new topic
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