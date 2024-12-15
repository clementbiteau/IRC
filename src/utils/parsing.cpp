# include "Server.hpp"

void Server::parseData(int fd, const std::string &data) {
    std::istringstream iss(data);
    std::string command;
    iss >> command;

    User *user = getUser(fd);
    if (!user) {
        ERR("451 :You're not registered\r\n");
        return;
    }

    // INITIAL registration -> obligatory for any safe IRC
    if (user->getIsAuth() && !user->getFlags().isRegistered) {
        if (command == "PASS" || command == "NICK" || command == "USER") {
        } else {
            sendErrorMessage(fd, "451 :You need to register first\r\n");
            return;
        }
    }

    if (command == "PASS") {
        pass(fd, iss, user);
    } else if (command == "NICK") {
        nick(fd, iss, user);
    } else if (command == "USER") {
        usering(fd, iss, user);
    } else if (command == "KICK") {
        kick(fd, iss, user);
    } else if (command == "INVITE") {
        invite(fd, iss, user);
    } else if (command == "TOPIC") {
        topic(fd, iss, user);
    } else if (command == "PING") {
        std::string output;
        iss >> output;
        sendMessage(fd, "PONG :" + output + "\r\n");
    } else if (command == "JOIN") {
        join(fd, iss, user);
    } else if (command == "QUIT") {
        sendMessage(fd, "Goodbye!\r\n");

        User *user = getUser(fd);
        if (user) {
         _disconnectedUsers.push_back(*user);
        }
        close(fd);
    } else if (command == "PRIVMSG") {
        std::string target;
        std::string message;
        iss >> target;
        std::getline(iss, message);
        if (!message.empty() && message[0] == ' ') {
            message.erase(0, 1);
        }
        privmsgCommand(fd, target, message);
    } else if (command == "CAP") {
        capls(fd);
    } else if (command == "WHOIS") {
        std::string nickname;
        iss >> nickname;
        if (nickname.empty())
            sendErrorMessage(fd, "431 :No nickname given\r\n");
        else
            whoisCommand(fd, user->getNickname(), nickname);
    } else if (command == "MODE") {
        mode(fd, iss);
    } else if (command == "WHOWAS") {
        std::string nickname;
        iss >> nickname;
        if (nickname.empty())
            sendErrorMessage(fd, "431 :No nickname given\r\n");
        else
            whowasCommand(fd, nickname);
    } else {
        if (!user->getFlags().isRegistered)
            sendErrorMessage(fd, "451 :You need to register first\r\n");
        else
            sendErrorMessage(fd, "421 " + command + " :Unknown command\r\n");
    }
}
