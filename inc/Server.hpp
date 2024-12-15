# include "Client.hpp"
# include "Channel.hpp"

class Server {
	private:
		int							_fd;
		int 						_port;
		std::string					_password;
		static bool					_signal;
		std::vector<Client>			_clients;
		std::vector<User>			_users;
		std::vector<Channel>		_channels;
		std::vector<struct pollfd>	_fds;
		std::vector<User>			_disconnectedUsers;

	public:
		Server(int port, std::string password);
		~Server();
		Server(Server const &src);
		Server &operator=(Server const &src);

		int				getPort();
		std::string		getPassword();
		Channel			*getChannel(std::string name);
		User			*getUser(int fd);
		std::string		getIp(int fd);
		int				getSameNicknameAmount(std::string nickname);
		User*			getUserByNickname(const std::string& nickname);
		std::string		getUserChannels(const User& user);


		void			init();
		void			createSocket();
		void			acceptNewClient();
		void			receiveData(int fd);
		void			parseData(int fd, const std::string &data);
		void			sendErrorMessage(int fd, std::string msg);
		void			sendMessage(int fd, std::string msg);
		void			sendMessageToUsers(Channel *chan, std::string authorName, std::string msg);
		void			createChannel(std::string channelName, User &user);
		void			sendMessageToAllUsers(const std::string& message);

		static void 	signalHandler(int signum);

		void			closeFds();
		void			clearClient(int fd);

		void			pass(int fd, std::istringstream &iss, User *user);
		void			nick(int fd, std::istringstream &iss, User *user);
		void			join(int fd, std::istringstream &iss, User *user);
		void			kick(int fd, std::istringstream &iss, User *user);
		void			invite(int fd, std::istringstream &iss, User *user);
		void			topic(int fd, std::istringstream &iss, User *user);
		void			mode(int fd, std::istringstream &iss);
		void			capls(int fd);
		void			usering(int fd, std::istringstream &iss, User *user);
		void			partCommand(int fd, std::string channelName, std::string message = "");
		void			privmsgCommand(int fd, std::string target, std::string message);
		void			whoisCommand(int fd, const std::string& requester, const std::string& nickname);
		void			whowasCommand(int fd, const std::string& nickname);
};