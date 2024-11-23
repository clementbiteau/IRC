#ifndef USER_HPP
#define USER_HPP

# include "defines.hpp"

class User {
	private:
		int			_fd;
		std::string _nickname;
		std::string _username;
		std::string _password;
		bool		_isOperator;
		flags		_flags;
		
	public:
		User();
		User(int fd, std::string nickname, std::string username, std::string password);
		~User();
		User(User const &src);
		User &operator=(User const &src);

		char		buffer[512];

		int			getFd() const;
		std::string	getNickname() const;
		std::string	getUsername() const;
		std::string	getPassword() const;
		flags		getFlags() const;
		bool		getIsOperator() const;

		void		setFd(int fd);
		void		setNickname(std::string nickname);
		void		setUsername(std::string username);
		void		setPassword(std::string password);
		void		setFlags(std::string flag, bool value);
		void		setIsOperator(bool isOperator);
};

#endif