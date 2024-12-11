# include "User.hpp"

User::User() {
	this->_nickname = "";
	this->_username = "";
	this->_password = "";
	this->_flags.first_join_ignore = false;
	this->_flags.isRegistered = false;
	this->_isAuth = false;
}

User::User(int fd, std::string nickname, std::string username, std::string password) {
	this->_fd = fd;
	this->_nickname = nickname;
	this->_username = username;
	this->_password = password;
	this->_flags.first_join_ignore = false;
	this->_flags.isRegistered = false;
	this->_isAuth = false;
}

User::~User() {}

User::User(User const &src) {
	if (this != &src)
		*this = src;
}

User &User::operator=(User const &src) {
	if (this != &src) {
		this->_fd = src._fd;
		this->_nickname = src._nickname;
		this->_username = src._username;
		this->_password = src._password;
		this->_flags = src._flags;
	}
	return *this;
}

int			User::getFd() const {
	return this->_fd;
}

std::string	User::getNickname() const {
	return this->_nickname;
}

std::string	User::getUsername() const {
	return this->_username;
}

std::string	User::getPassword() const {
	return this->_password;
}

flags	User::getFlags() const {
	return this->_flags;
}

bool User::getIsOperator() const {
    return this->_isOperator;
}

bool	User::getIsAuth() const {
	return this->_isAuth;
}

void User::setIsOperator(bool isOperator) {
    this->_isOperator = isOperator;
}

void		User::setFd(int fd) {
	this->_fd = fd;
}

void		User::setNickname(std::string nickname) {
	this->_nickname = nickname;
}

void		User::setUsername(std::string username) {
	this->_username = username;
}

void		User::setPassword(std::string password) {
	this->_password = password;
}

void		User::setFlags(std::string flag, bool value) {
	if (flag != "first_join_ignore" && flag != "isRegistered")
		return ;
	if (flag == "first_join_ignore")
		this->_flags.first_join_ignore = value;
	else if (flag == "isRegistered")
		this->_flags.isRegistered = value;
}

void	User::setIsAuth(bool status) {
	this->_isAuth = status;
}