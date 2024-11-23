#include "Utils.hpp"

static bool checkPort(int port) {
	if (port < 1024 || port > 49151) {
		ERR("Invalid port number. He must be between 1024 and 49151.");
		return false;
	}
	return true;
}

static bool checkPassword(std::string password) {
	if (password.empty()) {
		ERR("Invalid password. He must not be empty.");
		return false;
	}
	return true;
}

bool	check(int port, std::string password) {
	if (!checkPort(port) || !checkPassword(password))
		return false;
	return true;
}