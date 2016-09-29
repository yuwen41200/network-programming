#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

void run(string cmd, string data, string token);

int main(int argc, char **argv) {
	if (argc != 3)
		cout << "usage: " << argv[0] << " [input file] [split token]" << endl;

	ifstream input(argv[1]);

	if (!input.is_open()) {
		cout << "error: cannot open " << argv[1] << endl;
		exit(0);
	}

	string cmd, data, token = argv[2];
	char ws;

	while (!input.eof()) {
		input >> cmd >> ws;
		getline(input, data);
		run(cmd, data, token);
	}

	input.close();

	while (true) {
		cin >> cmd >> ws;
		getline(cin, data);
		run(cmd, data, token);
	}
}

void run(string cmd, string data, string token) {
	if (cmd == "reverse") {
		reverse(data.begin(), data.end());
		cout << data << endl;
	}

	else if (cmd == "split") {
		size_t len = data.length();
		char cstr[len], *result;
		data.copy(cstr, len);
		cstr[len] = '\0';
		result = strtok(cstr, token.c_str());
		while (result) {
			cout << result << " ";
			result = strtok(NULL, token.c_str());
		}
		cout << endl;
	}

	else if (cmd == "exit")
		exit(0);

	else
		cout << "error: invalid command" << endl;
}
