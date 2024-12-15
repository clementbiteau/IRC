# include "Utils.hpp"
# include "Server.hpp"

void	Server::capls(int fd) {
	sendMessage(fd, "CAP * LS :PASS NICK USER JOIN PART LIST PRIVMSG NOTICE MODE KICK INVITE TOPIC PING QUIT\r\n");
}

void Server::pass(int fd, std::istringstream &iss, User *user) {
    std::string password;

    iss >> password;

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

    if (user->getNickname().empty()) {
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

    std::cout << "User " << user->getNickname() << " attempting to join channel " << channel << "..." << std::endl;

    Channel *chan = getChannel(channel);

    if (chan == NULL) {
        createChannel(channel, *user);
        chan = getChannel(channel);
        if (chan) {
            chan->addUser(*user);
            chan->addOperator(user->getNickname());

            std::cout << "User " << user->getNickname() << " created and joined channel " << channel << " as an operator." << std::endl;
            std::string notification = user->getNickname() + " has created and joined the channel as an operator.";
            chan->sendMessageToChannel(notification, user->getNickname());
        } else {
            std::cerr << "Error: Failed to create channel " << channel << "." << std::endl;
            sendErrorMessage(fd, "403 " + channel + " :Failed to create channel\r\n");
        }
        return;
    }

    std::cout << "Channel " << channel << " exists, checking user limit and password." << std::endl;

    if (chan->getUserLimit() > 0 && chan->getMembersList().size() >= chan->getUserLimit()) {
        sendErrorMessage(fd, "471 " + channel + " :Cannot join channel (+l)\r\n");
        std::cout << "User: " << user->getNickname() << " failed to join channel #" << channel << " due to user limit (+l)" << std::endl;
        return;
    }

    if (chan->getChannelPassword() != "" && password != chan->getChannelPassword()) {
        sendErrorMessage(fd, "475 " + channel + " :Bad channel password\r\n");
        std::cout << "User: " << user->getNickname() << " failed to join channel #" << channel << " due to incorrect password" << std::endl;
        return;
    }

    if (chan->isInviteOnly() && !chan->isUserInvited(user->getNickname())) {
        sendErrorMessage(fd, "473 " + channel + " :Cannot join channel (+i)\r\n");
        std::cout << "User: " << user->getNickname() << " is not invited to join channel #" << channel << std::endl;
        return;
    }

    if (chan->isUserInChannel(*user)) {
        sendErrorMessage(fd, "443 " + channel + " :is already a member of the channel\r\n");
        std::cout << "User: " << user->getNickname() << " is already a member of channel #" << channel << std::endl;
        return;
    }

    chan->addUser(*user);
    std::string notification = user->getNickname() + " has joined the channel.";
    chan->sendMessageToChannel(notification, user->getNickname());

    std::string joinMessage = RPL_TOPIC(user->getNickname(), chan->getChannelName(), chan->getChannelTopic()) + 
                               RPL_NAMREPLY(user->getNickname(), chan->getChannelName(), chan->getMembersListNames()) + 
                               RPL_ENDOFNAMES(user->getNickname(), chan->getChannelName());
    sendMessage(fd, joinMessage);
    std::cout << "User " << user->getNickname() << " successfully joined channel " << channel << std::endl;
}

void Server::kick(int fd, std::istringstream &iss, User *user) {
    std::string channelName;
    std::string userToKickNick;
    std::string reason;

    iss >> channelName >> userToKickNick;
    std::getline(iss, reason);
    reason = reason.empty() ? "No reason given" : reason.substr(1);

    if (channelName.empty() || userToKickNick.empty()) {
        sendErrorMessage(fd, "461 KICK :Not enough parameters\r\n");
        return;
    }

    Channel *chan = getChannel(channelName);
    if (chan == nullptr) {
        sendErrorMessage(fd, "403 " + channelName + " :No such channel\r\n");
        return;
    }

    if (!chan->isUserInChannel(*user)) {
        sendErrorMessage(fd, "442 " + channelName + " :You're not on that channel\r\n");
        return;
    }

    if (!chan->isOperator(*user)) {
        sendErrorMessage(fd, "482 " + channelName + " :You're not a channel operator\r\n");
        return;
    }

    User *userToKick = chan->getUserByNickname(userToKickNick);
    if (userToKick == nullptr) {
        sendErrorMessage(fd, "441 " + userToKickNick + " " + channelName + " :They aren't on that channel\r\n");
        return;
    }

    sendMessage(userToKick->getFd(), ":" + user->getNickname() + " kicked you out from: " + channelName + " dear " + userToKickNick + "... Cause: " + reason + "\r\n");

    chan->removeUser(*userToKick);
    std::string notification = ":" + user->getNickname() + " KICK " + channelName + " " + userToKickNick + " :" + reason + "\r\n";
    chan->sendMessageToChannel(notification, user->getNickname());

    std::cout << "User " << userToKickNick << " was kicked from channel " << channelName
              << " by " << user->getNickname() << " (" << reason << ")" << std::endl;
}

void Server::invite(int fd, std::istringstream &iss, User *user) {
    std::string inviteeNickname;
    std::string channelName;

    iss >> inviteeNickname;
    iss >> channelName;

    if (inviteeNickname.empty() || channelName.empty()) {
        sendErrorMessage(fd, "461 INVITE :Not enough parameters\r\n");
        return;
    }

    Channel *chan = getChannel(channelName);
    if (chan == NULL) {
        sendErrorMessage(fd, "403 " + channelName + " :No such channel\r\n");
        return;
    }

    if (!chan->isOperator(*user)) {
        sendErrorMessage(fd, "482 " + channelName + " :You're not a channel operator\r\n");
        return;
    }

    User *invitee = getUserByNickname(inviteeNickname);
    if (invitee == NULL) {
        sendErrorMessage(fd, "401 " + inviteeNickname + " :No such nick\r\n");
        return;
    }

    if (chan->isUserInChannel(*invitee)) {
        sendErrorMessage(fd, "443 " + inviteeNickname + " " + channelName + " :is already on channel\r\n");
        return;
    }

    chan->addInvitedUser(inviteeNickname);
    sendMessage(invitee->getFd(), ":" + user->getNickname() + " has invited you dear " + inviteeNickname + " to join the channel -> " + channelName + "\r\n");
    sendMessage(fd, "You: " + user->getNickname() + " have invited " + inviteeNickname + " to " + channelName + "\r\n");
    std::cout << "User " << user->getNickname() << " invited " << inviteeNickname << " to channel " << channelName << std::endl;
}

void Server::topic(int fd, std::istringstream &iss, User *user) {
    std::string channel;
    std::string topic;

    iss >> channel;

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

    if (chan->isTopicRestricted() && !chan->isOperator(*user)) {
        sendErrorMessage(fd, "482 " + channel + " :You're not a channel operator\r\n");
        return;
    }

    chan->setChannelTopic(topic);
    sendMessage(fd, "332 " + channel + " :" + topic + "\r\n");
}

void Server::mode(int fd, std::istringstream &iss) {
    std::string channel;
    std::string modeString;
    std::string param;

    iss >> channel;
    channel = trim(channel);

    if (channel.empty()) {
        sendErrorMessage(fd, "461 :Invalid channel name\r\n");
        return;
    }

    iss >> modeString;
    modeString = trim(modeString);

    if (modeString.empty() || (modeString[0] != '+' && modeString[0] != '-')) {
        sendErrorMessage(fd, "461 " + channel + " :Invalid MODE command\r\n");
        return;
    }

    if (iss) {
        std::getline(iss, param);
        param = trim(param);
    }

    std::cout << "Mode String: " << modeString << ", Parameter: " << param << std::endl;

    Channel *chan = getChannel(channel);
    if (!chan) {
        sendErrorMessage(fd, "403 " + channel + " :No such channel\r\n");
        return;
    }

    User *user = getUser(fd);
    if (!user || !chan->isOperator(*user)) {
        sendErrorMessage(fd, "482 " + channel + " :You're not a channel operator\r\n");
        return;
    }

    if (modeString.size() < 2) {
        sendErrorMessage(fd, "461 " + channel + " :Invalid MODE command\r\n");
        return;
    }

    bool setMode = (modeString[0] == '+');
    char modeChar = modeString[1];

    switch (modeChar) {
        case 'i': // Invite-only
            chan->setInviteOnly(setMode);
            break;

        case 't': // Topic restriction
            chan->setTopicRestriction(setMode);
            break;

        case 'k': // Channel key (password)
            if (setMode) {
                if (param.empty()) {
                    sendErrorMessage(fd, "461 " + channel + " :Password required for +k\r\n");
                    return;
                }
                chan->setPassword(param);
            } else {
                chan->clearPassword();
            }
            break;

        case 'o': // Channel operator
            if (setMode) {
                if (param.empty()) {
                    sendErrorMessage(fd, "461 " + channel + " :Nickname required for +o\r\n");
                    return;
                }
                if (!chan->addOperator(param)) {
                    sendErrorMessage(fd, "441 " + param + " " + channel + " :User not in channel\r\n");
                }
            } else {
                if (param.empty()) {
                    sendErrorMessage(fd, "461 " + channel + " :Nickname required for -o\r\n");
                    return;
                }
                chan->removeOperator(param);
            }
            break;

        case 'l': // User limit
            if (setMode) {
                int limit = atoi(param.c_str());
                if (limit <= 0) {
                    sendErrorMessage(fd, "461 " + channel + " :Invalid user limit\r\n");
                    return;
                }
                chan->setUserLimit(limit);
            } else {
                chan->clearUserLimit();
            }
            break;

        default:
            sendErrorMessage(fd, "472 " + std::string(1, modeChar) + " :is unknown mode char to me\r\n");
            return;
    }

    std::string modeResponse = "324 " + channel + " " + modeString + "\r\n";
    sendMessage(fd, modeResponse);

    const std::vector<User>& members = chan->getMembersList();
    for (size_t i = 0; i < members.size(); ++i) {
        if (members[i].getFd() != fd) {
            sendMessage(members[i].getFd(), modeResponse);
        }
    }
}
