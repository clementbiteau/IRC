#ifndef USER_HPP
#define USER_HPP

# include "defines.hpp"

class User {
	private:
		int			_fd;
		std::string _nickname;
		std::string _username;
		std::string _password;
		std::string	_realname;
		bool		_isOperator;
		bool		_isAuth;
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
		std::string	getRealName() const;
		flags		getFlags() const;
		bool		getIsOperator() const;
		bool		getIsAuth() const;


		void		setFd(int fd);
		void		setNickname(std::string nickname);
		void		setUsername(std::string username);
		void		setPassword(std::string password);
		void		setFlags(std::string flag, bool value);
		void		setIsOperator(bool isOperator);
		void		setIsAuth(bool status);
		void		setRealName(std::string realname);
};

#endif