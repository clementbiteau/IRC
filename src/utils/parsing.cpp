# include "Server.hpp"

// void Server::parseData(int fd, std::string data) {
//     std::string delimiter = "\r\n";
//     size_t pos = 0;
//     std::string line;

//     while ((pos = data.find(delimiter)) != std::string::npos) {
//         line = data.substr(0, pos);
//         data.erase(0, pos + delimiter.length());

//         if (!line.empty()) {
//             std::istringstream iss(line);
//             std::string command;
//             iss >> command;

// 			User *user = getUser(fd);
//             if (!user) {
//                 sendErrorMessage(fd, "451 :You're not registered\r\n");
//                 return ;
//             }
// 			if (command == "PASS") {
//                 if (user != NULL)
// 				    pass(fd, iss, user);
// 			} else if (command == "NICK") {
//                 nick(fd, iss, user);
//             } else if (command == "USER") {
//                 std::string output;
//                 iss >> output;
//                 user->setUsername(output);
//             } else if (command == "JOIN") {
//                 join(fd, iss, user);
//             } else if (command == "KICK") {
//                 kick(fd, iss, user);
//             } else if (command == "INVITE") {
//                 invite(fd, iss, user);
//             } else if (command == "TOPIC") {
//                 topic(fd, iss);
//             } else if (command == "PING") {
//                 std::string output;
//                 iss >> output;
//                 sendMessage(fd, "PONG :" + output + "\r\n");
//             } else if (command == "QUIT") {
//                 sendMessage(fd, "Goodbye!\r\n");
//                 close(fd);
// 				exit(0);
//             } else if (command == "CAP") {
// 				capls(fd);
// 			}
//             else if (command == "MODE") {
//                 mode(fd, iss);
//             }

// 		}
//     }
// }

void Server::parseData(int fd, const std::string &data) {
    std::istringstream iss(data);
    std::string command;
    iss >> command;

    User *user = getUser(fd);
    if (!user) {
        ERR("451 :You're not registered\r\n");
        return;
    }

    if (command == "PASS") {
        pass(fd, iss, user);
    } else if (command == "NICK" && user->getFlags().isRegistered) {
        nick(fd, iss, user);
    } else if (command == "KICK") {
        kick(fd, iss, user);
    } else if (command == "USER") {
        std::string output;
        iss >> output;
        user->setUsername(output);
    } else if (command == "INVITE") {
        invite(fd, iss, user);
    } else if (command == "TOPIC") {
        topic(fd, iss);
    } else if (command == "PING") {
        std::string output;
        iss >> output;
        sendMessage(fd, "PONG :" + output + "\r\n");
    } else if (command == "JOIN") {
        join(fd, iss, user);
    } else if (command == "QUIT") {
        sendMessage(fd, "Goodbye!\r\n");
        close(fd);
    }  else if (command == "CAP") {
        capls(fd);
	} else if (command == "MODE") {
        mode(fd, iss);
    }
    else {
        if (!user->getFlags().isRegistered)
            sendErrorMessage(fd, "451 :You need to register first\r\n");
        else
            sendErrorMessage(fd, "421 " + command + " :Unknown command\r\n");
    }
}
