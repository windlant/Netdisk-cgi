#include <tools.h>
#include <json.hpp>
#include <iostream>
using namespace std;
using json_t = nlohmann::json;

int main()
{
	json_t req = get_req();
	string uname = req["username"];
	string pwd = req["pwd"];
	json_t res = win_register(uname, pwd);
	send_json(res);

	return 0;
}