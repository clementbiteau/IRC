# include "Channel.hpp"

Channel::Channel() {}

Channel::Channel(std::string name, std::string password)
{
	_name = name;
	_password = password;
}

Channel::~Channel() {}

Channel::Channel(Channel const &src)
{
	if (this != &src)
		*this = src;
}

Channel &Channel::operator=(Channel const &src)
{
	if (this != &src)
	{
		_name = src._name;
		_topic = src._topic;
		_password = src._password;
		_users = src._users;
		_operators = src._operators;
		_messageHistory = src._messageHistory;
	}
	return *this;
}

bool	Channel::operator==(const Channel& other) const {
    return this->_name == other._name;
}

const std::string &Channel::getChannelName() const
{
	return this->_name;
}

const std::string &Channel::getChannelTopic() const
{
	return this->_topic;
}

const std::string &Channel::getChannelPassword() const
{
	return this->_password;
}

const User &Channel::getChannelOwner() const
{
	return this->_users[0];
}

const std::vector<User> &Channel::getMembersList() const
{
	return this->_users;
}

const std::string Channel::getMembersListNames() const
{
	std::string names;

	for (size_t i = 0; i < _users.size(); i++)
	{
		names += _users[i].getNickname();
		if (i < _users.size() - 1)
			names += " ";
	}
	return names;
}

User* Channel::getUserByNickname(const std::string& nickname)
{
    for (size_t i = 0; i < _users.size(); ++i)
    {
        if (_users[i].getNickname() == nickname)
        {
            return &_users[i];
        }
    }
    return nullptr;
}

bool Channel::isUserInChannel(const User &user) const
{
    for (size_t i = 0; i < _users.size(); ++i)
    {
        if (_users[i].getNickname() == user.getNickname())
            return true;
    }
    return false;
}

const std::vector<std::string> &Channel::getModesList() const
{
	return this->_modes;
}

const std::vector<std::string> &Channel::getMessageHistory() const
{
	return this->_messageHistory;
}

void Channel::setChannelName(std::string name)
{
	this->_name = name;
}

void Channel::setChannelTopic(std::string topic)
{
	this->_topic = topic;
}

void Channel::setChannelOwner(User owner)
{
	this->_users.push_back(owner);
}

void Channel::setMembersList(std::vector<User> members)
{
	this->_users = members;
}

void Channel::setModesList(std::vector<std::string> modes)
{
	this->_modes = modes;
}

void Channel::setMessageHistory(std::vector<std::string> history)
{
	this->_messageHistory = history;
}

void Channel::addMode(std::string mode)
{
	this->_modes.push_back(mode);
}

void Channel::removeMode(std::string mode)
{
	for (size_t i = 0; i < this->_modes.size(); ++i)
	{
		if (this->_modes[i] == mode)
		{
			this->_modes.erase(this->_modes.begin() + i);
			return ;
		}
	}
}

void Channel::addUser(User& user) {
    for (size_t i = 0; i < _users.size(); i++) {
        if (_users[i].getNickname() == user.getNickname()) {
            std::cout << "Warning: User " << user.getNickname() << " is already in the channel." << std::endl;
            return;
        }
    }
    _users.push_back(user);
}


void Channel::removeUser(User user)
{
	for (size_t i = 0; i < this->_users.size(); ++i)
	{
		if (this->_users[i].getNickname() == user.getNickname())
		{
			this->_users.erase(this->_users.begin() + i);
			return ;
		}
	}
}

bool Channel::isOperator(const User &user) const {
    for (std::vector<User>::const_iterator it = _operators.begin(); it != _operators.end(); ++it) {
        if (it->getNickname() == user.getNickname()) {
            return true;
        }
    }
    return false;
}

void Channel::addOperator(User user)
{
    if (!isOperator(user)) {
        _operators.push_back(user);
    }
}

void Channel::removeOperator(User user)
{
    for (std::vector<User>::iterator it = _operators.begin(); it != _operators.end(); ++it) 
	{
        if (it->getNickname() == user.getNickname()) 
		{
            _operators.erase(it);
            break;
        }
    }
}

void Channel::sendMessageToChannel(std::string message, std::string author)
{
	std::string msg = ":" + author + " PRIVMSG " + this->getChannelName() + " :" + message + "\r\n";

	std::cout << "author: " << author << std::endl;
	for (size_t i = 0; i < _users.size(); i++)
	{
		std::cout << "user " << i << " : " << _users[i].getNickname() << std::endl;
		if (_users[i].getNickname() != author)
		{
			int bytes = send(_users[i].getFd(), msg.c_str(), msg.size(), MSG_DONTWAIT); 
			if (bytes <= 0)
			{
				std::cout << "Notice : User [" << _users[i].getNickname() << "] cannot be reached and will be disconnected from the server." << std::endl;
				continue;
			}
			usleep(50000);
		}
	}
	addMessageToHistory(msg);
	return;
}

void Channel::addMessageToHistory(std::string message)
{
	_messageHistory.push_back(message);
}

std::ostream    &operator<<(std::ostream &flux, const Channel& src)
{
	flux << "Channel Name: " << src.getChannelName() << std::endl;
	flux << "Channel Topic: " << src.getChannelTopic() << std::endl;
	flux << "Channel Owner: " << src.getChannelOwner().getNickname() << std::endl;

	flux << "Members List: ";
	for (size_t i = 0; i < src.getMembersList().size(); ++i)
	{
		flux << src.getMembersList()[i].getNickname() << ":" << src.getMembersList()[i].getFd();
		if (i < src.getMembersList().size() - 1)
			flux << ", ";
	}
	flux << std::endl;

	flux << "Modes List: ";
	for (size_t i = 0; i < src.getModesList().size(); ++i)
	{
		flux << src.getModesList()[i];
		if (i < src.getModesList().size() - 1)
			flux << ", ";
	}
	flux << std::endl;

	flux << "Message History: ";
	for (size_t i = 0; i < src.getMessageHistory().size(); ++i)
	{
		flux << src.getMessageHistory()[i];
		if (i < src.getMessageHistory().size() - 1)
			flux << ", ";
	}
	flux << std::endl;
	return flux;
}
