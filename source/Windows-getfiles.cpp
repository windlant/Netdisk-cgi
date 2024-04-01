#include <tools.h>
#include <json.hpp>
#include <iostream>
using namespace std;
using json_t = nlohmann::json;

int main()
{
	string log;
	json_t req = get_req();
	string uname = req["username"];
	string token = req["token"];
	json_t res = get_all_files(uname, token);
	send_json(res);
	
	return 0;
}