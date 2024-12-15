# include "defines.hpp"
# include "User.hpp"

class Channel {
	private:
		std::string					_name;
		std::string					_topic;
		std::string					_password;
		std::vector<User>			_users;
		std::vector<User>			_operators;
		std::vector<std::string>	_modes;
		std::vector<std::string>	_messageHistory;
		std::unordered_set<std::string> _invitedUsers;
		bool						_inviteOnly;
		bool						_topicRestricted;
		int							_userLimit;

	public:
		Channel();
		Channel(std::string name, std::string password);
		~Channel();
		Channel(Channel const &src);
		Channel &operator=(Channel const &src);

		bool	operator==(const Channel& other) const;

		const std::string					&getChannelName() const ;
		const std::string					&getChannelTopic() const ;
		const std::string					&getChannelPassword() const ;
		const User							&getChannelOwner() const ;
		const std::vector<User>				&getMembersList() const ;
		const std::string					getMembersListNames() const ;
		const std::vector<std::string>		&getModesList() const ;
		const std::vector<std::string>		&getMessageHistory() const ;
		User*								getUserByNickname(const std::string& nickname);
		bool								isUserInChannel(const User& user) const;
		bool								isOperator(const std::string& nickname) const;
		bool								isOperator(const User& user) const;


		void								setChannelName(std::string name);
		void								setChannelTopic(std::string topic);
		void								setChannelOwner(User owner);
		void								setMembersList(std::vector<User> members);
		void								setMessageHistory(std::vector<std::string> history);
		void								setModesList(std::vector<std::string> modes);

		bool                               isInviteOnly() const;
        void                               setInviteOnly(bool enable);
        bool                               isTopicRestricted() const;
        void                               setTopicRestriction(bool enable);
        size_t                                getUserLimit() const;
        void                               setUserLimit(int limit);
        void                               clearUserLimit();

		bool 								isUserInvited(const std::string& nickname);
        void 								addInvitedUser(const std::string& nickname);
        void 								removeInvitedUser(const std::string& nickname);

        bool                               hasPassword() const;
        void                               setPassword(const std::string& password);
        void                               clearPassword();
		
		
		void								addMode(std::string mode);
		void								removeMode(std::string mode);
		void								addUser(User &user);
		void								removeUser(User user);
		bool								addOperator(const std::string& nickname);
		void								removeOperator(const std::string& nickname);
		void								sendMessageToChannel(std::string message, std::string author);
		void								sendMessageToChannelPrv(std::string message, std::string author, int senderFd);
		void								addMessageToHistory(std::string message);

};

std::ostream               &operator<<(std::ostream &flux, const Channel& src);