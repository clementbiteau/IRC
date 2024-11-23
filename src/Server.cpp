# include "Server.hpp"

bool	Server::_signal = false;
Server::Server(int port, std::string password) {
	_port = port;
	_password = password;
}

Server::~Server() {}

Server::Server(Server const &src) {
	if (this != &src)
		*this = src;
}

Server &Server::operator=(Server const &src) {
	if (this != &src) {
		this->_port = src._port;
		this->_password = src._password;
	}
	return *this;
}

int	Server::getPort() {
	return this->_port;
}

std::string	Server::getPassword() {
	return this->_password;
}

Channel *Server::getChannel(std::string name) {
	for (size_t i = 0; i < _channels.size(); i++) {
		if (_channels[i].getChannelName() == name)
			return &_channels[i];
	}
	return NULL;
}

User	*Server::getUser(int fd) {
	for (size_t i = 0; i < _users.size(); i++) {
		if (_users[i].getFd() == fd)
			return &_users[i];
	}
	return NULL;
}

std::string	Server::getIp(int fd) {
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);

    if (getpeername(fd, (struct sockaddr*)&addr, &addrLen) == -1) {
        return "Unknown";
    }
    return std::string(inet_ntoa(addr.sin_addr));
}

int	Server::getSameNicknameAmount(std::string nickname) {
	int	count = 0;
	for (size_t i = 0; i < _users.size(); i++) {
		std::cout << "fd: " << _users[i].getFd() << " | nickname: " << _users[i].getNickname() << std::endl;
		if (_users[i].getNickname() == nickname)
			count++;
	}
	return count;
}

void	Server::init() {
	signal(SIGINT, signalHandler);
	signal(SIGQUIT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGPIPE, signalHandler);
	createSocket();

	while (Server::_signal == false)
	{
		if((poll(&_fds[0], _fds.size(), -1) == -1) && Server::_signal == false)
			throw(std::runtime_error("poll() faild"));

		for (size_t i = 0; i < _fds.size(); i++)
		{
			if (_fds[i].revents & POLLIN)
			{
				if (_fds[i].fd == _fd)
					acceptNewClient();
				else
					receiveData(_fds[i].fd);
			}
		}
	}
	closeFds();
}

void	Server::createSocket() {
	struct sockaddr_in add;
	struct pollfd newPoll;

	add.sin_family = AF_INET;
	add.sin_port = htons(this->_port);
	add.sin_addr.s_addr = INADDR_ANY;

	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(_fd == -1)
		throw(std::runtime_error("faild to create socket"));

	int en = 1;
	if(setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	if (fcntl(_fd, F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(_fd, (struct sockaddr *)&add, sizeof(add)) == -1)
		throw(std::runtime_error("faild to bind socket"));
	if (listen(_fd, SOMAXCONN) == -1)
		throw(std::runtime_error("listen() faild"));

	newPoll.fd = _fd;
	newPoll.events = POLLIN;
	newPoll.revents = 0;
	_fds.push_back(newPoll);

	MSG(GREEN, "Server", "Connected !");
	std::cout << "Waiting for clients..." << std::endl;
}

void	Server::sendErrorMessage(int fd, std::string msg) {
	const std::string errorMsg = std::string(RED) + "[ERROR] " + WHITE + msg + "\r\n";
	sendMessage(fd, errorMsg);
}

void	Server::sendMessage(int fd, std::string msg) {
	int 		bytes;
	int 		dataSent = 0;
	
	if (msg.size() > 512) {
		std::cout << "Warning : Message sent is more than 512 characters, truncating the result" << std::endl;
		msg = msg.substr(0, 510);
		msg += "\r\n";
		std::cout << "Warning : message is now [" << msg << "]" << std::endl;
	}
	
	while (size_t(dataSent) < msg.size())
	{
		if ((bytes = send(fd, msg.c_str() + dataSent, msg.size() - dataSent, MSG_DONTWAIT)) <= 0)
		{
			sendErrorMessage(fd, "451 :Client where the message is supposed to be sent is not registered.\r\n");
			return ;
		}
		dataSent += bytes;
	}
}

void	Server::sendMessageToUsers(Channel *chan, std::string authorName, std::string msg) {
	std::string message = ":" + authorName + " PRIVMSG " + chan->getChannelName() + " :" + msg + "\r\n";
	
	for (size_t i = 0; i < _users.size(); i++)
	{
		if (_users[i].getNickname() != authorName)
		{
			int bytes = send(_users[i].getFd(), message.c_str(), message.size(), MSG_DONTWAIT); 
			if (bytes <= 0)
			{
				std::cout << "Notice : User [" << _users[i].getNickname() << "] cannot be reached and will be disconnected from the server." << std::endl;				continue ;
			}
			usleep(50000);
		}
	}
}

void	Server::createChannel(std::string channelName, User &user) {
	Channel newChannel(channelName, "");
	_channels.push_back(newChannel);
	newChannel.addUser(user);
	// sendMessageToUsers(&newChannel, user.getNickname(), RPL_JOIN(USER(user.getNickname(), user.getUsername()), newChannel.getChannelName()));
	std::string joinMessage = RPL_TOPIC(user.getNickname(), newChannel.getChannelName(), newChannel.getChannelTopic()) + 
									RPL_NAMREPLY(user.getNickname(), newChannel.getChannelName(), newChannel.getMembersListNames())	+ 
									RPL_ENDOFNAMES(user.getNickname(), newChannel.getChannelName());
	sendMessage(user.getFd(), joinMessage);
}

void	Server::acceptNewClient() {
	Client client;
	struct sockaddr_in newClient;
	struct pollfd newPoll;
	socklen_t len = sizeof(newClient);

	int newClientFd = accept(_fd, (sockaddr *)&(newClient), &len);
	if (newClientFd == -1) {
		ERR("accept() failed");
		return;
	}

	if (fcntl(newClientFd, F_SETFL, O_NONBLOCK) == -1) {
		ERR("fcntl() failed");
		return;
	}

	newPoll.fd = newClientFd;
	newPoll.events = POLLIN;
	newPoll.revents = 0;

	client.setFd(newClientFd);
	client.setIp(inet_ntoa((newClient.sin_addr)));
	_clients.push_back(client);
	_fds.push_back(newPoll);

	User newUser(newClientFd, "", "", "");
	_users.push_back(newUser);

	client.setUser(&newUser);

	CLIENT_MSG(GREEN, "Client", "Client ", newClientFd, " is connected !");
	// sendMessage(newClientFd, "001 : Welcome to the server !\r\n");
}

void	Server::receiveData(int fd) {
	char buff[1024];
	memset(buff, 0, sizeof(buff));

	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);

	if (bytes <= 0) {
		CLIENT_MSG(RED, "Client", "Client ", fd, " has been disconnected.");
		clearClient(fd);
		close(fd);
	} else {
		buff[bytes] = '\0';
		CLIENT_MSG(BLUE, "Client", "Data received: ", fd, buff);
		// std::cout << "\n-------------------\nraw data:\n" << buff << "\n------------------------\n" << std::endl;
		
		User *user = getUser(fd);
		if (!user)
			return ;
		
		parseData(fd, buff);
	}
}

void	Server::signalHandler(int signum) {
	if (signum == SIGPIPE)
		return ;
	_signal = true;
}

void	Server::closeFds() {
	for (size_t i = 0; i < _clients.size(); i++) {
		// CLIENT_MSG(RED, "Client", "Client" , _clients[i].getFd(), " has been disconnected.");
		close(_clients[i].getFd());
	}
	if (_fd != -1){
		// MSG(RED, "Server", "Disconnected.");
		close(_fd);
	}
}

void Server::clearClient(int fd) {
	// Save info of user before deletion
    for (size_t i = 0; i < _clients.size(); i++) {
        if (_clients[i].getFd() == fd)
		{
            _disconnectedUsers.push_back(_clients[i].getUser());
            close(fd);
            _clients.erase(_clients.begin() + i);
            break;
        }
    }
    for (size_t i = 0; i < _fds.size(); i++) {
        if (_fds[i].fd == fd) {
            _fds.erase(_fds.begin() + i);
            break;
        }
    }
}

void Server::partCommand(int fd, std::string channelName, std::string message) {
    Channel *channel = getChannel(channelName);
    if (!channel) {
        sendErrorMessage(fd, "403: No such channel.");
        return;
    }
    User *user = getUser(fd);
    if (!user) {
        sendErrorMessage(fd, "401: No such nick.");
        return;
    }
    channel->removeUser(*user);
    std::string partMsg = ":" + user->getNickname() + " PART " + channelName + " :" + message + "\r\n";
    channel->sendMessageToChannel(partMsg, user->getNickname());
    
    if (channel->getMembersList().empty())
    {
        std::vector<Channel>::iterator it = std::find(_channels.begin(), _channels.end(), *channel);
        if (it != _channels.end())
            _channels.erase(it);
    }
}

void Server::privmsgCommand(int fd, std::string target, std::string message) {
    User *user = getUser(fd);
    if (!user) {
        sendErrorMessage(fd, "401: No such nick.");
        return;
    }
    if (target[0] == '#') {
        Channel *channel = getChannel(target);
        if (!channel) {
            sendErrorMessage(fd, "403: No such channel.");
            return;
        }
        channel->sendMessageToChannel(message, user->getNickname());
    }
	else
	{
        for (size_t i = 0; i < _users.size(); i++) {
            if (_users[i].getNickname() == target) {
                sendMessage(_users[i].getFd(), ":" + user->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n");
                return;
            }
        }
        sendErrorMessage(fd, "401: No such nick.");
    }
}

void Server::whoisCommand(int fd, std::string nickname) {
    User *user = nullptr;
    for (size_t i = 0; i < _users.size(); i++) {
        if (_users[i].getNickname() == nickname) {
            user = &_users[i];
            break;
        }
    }
    if (!user) {
        sendErrorMessage(fd, "401: No such nick.");
        return;
    }
    std::string whoisResponse = "311 " + nickname + " " + user->getUsername() + " :User info\r\n";
    sendMessage(fd, whoisResponse);
}

void Server::whowasCommand(int fd, std::string nickname) {
    for (size_t i = 0; i < _disconnectedUsers.size(); i++) {
        if (_disconnectedUsers[i].getNickname() == nickname) {
            std::string whowasResponse = "314 " + nickname + " " + _disconnectedUsers[i].getUsername() + " :User history\r\n";
            sendMessage(fd, whowasResponse);
            return;
        }
    }
    sendErrorMessage(fd, "406: There was no history for that nickname.");
}