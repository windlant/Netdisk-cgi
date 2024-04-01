#include <tools.h>
#include <json.hpp>
#include <iostream>
using namespace std;
using json_t = nlohmann::json;

int if_tid(json_t req)
{
	string json_str = req.dump();
	if(json_str.find("\"tid\"") != string::npos)
		return 1;
	else
		return 0;
}

int main()
{
	string log;
	// log = "开始";
	// add_log(log);

	json_t req = get_req();
	json_t res;
	res["result"] = -1;

	if(!if_tid(req))
		req["tid"] = -1;
	string task_type = req["type"];
	if(task_type == "file_up")
	{
		res = win_create_file_up_task(req);
		send_json(res);
		return 0;
	}
	else if(task_type == "file_down")
	{
		// log = "开始创建下载任务";
		// add_log(log);
		res = win_create_file_down_task(req);
		send_json(res);
		return 0;
	}
	return 0;
}