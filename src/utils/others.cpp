# include "Utils.hpp"

int	atoi(char *str) {
	int		result = 0;
	int		sign = 1;
	int		i = 0;

	if (str[i] == '-') {
		sign = -1;
		i++;
	}
	while (str[i] >= '0' && str[i] <= '9') {
		result = result * 10 + str[i] - '0';
		i++;
	}
	return (result * sign);
}

std::string itoa(int nb) {
	std::string str;
	std::string result;

	while (nb > 0) {
		str += nb % 10 + '0';
		nb /= 10;
	}
	for (int i = str.size() - 1; i >= 0; i--)
		result += str[i];
	return result;
}

std::string trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    if (first == std::string::npos || last == std::string::npos)
        return "";
    return str.substr(first, last - first + 1);
}