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