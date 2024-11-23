#include "Server.hpp"
#include "Utils.hpp"
#include "defines.hpp"

int	main(int ac, char **av)
{
	if (ac != 3) {
		ERR("Usage: ./ircserv [port] [password]");
		return (1);
	}

	if (!check(atoi(av[1]), av[2]))
		return (1);

	Server	server(atoi(av[1]), av[2]);

	try {
		server.init();
	} catch (std::exception &e) {
		ERR(e.what());
		server.closeFds();
		return (1);
	}

	MSG(RED, "Server", "Disconnected.");
	return (0);
}