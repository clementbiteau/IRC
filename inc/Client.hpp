# include "defines.hpp"
# include "User.hpp"

class Client {
	private:
		int			_fd;
		std::string	_ip;
		User		_user;

	public:
		Client();
		~Client();
		Client(Client const &src);
		Client &operator=(Client const &src);

		int			getFd();
		std::string	getIp();
		User		getUser();

		void		setFd(int fd);
		void		setIp(std::string ip);
		void		setUser(User *user);
};