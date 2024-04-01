#include <tools.h>
#include <json.hpp>
#include <iostream>
using namespace std;
using json_t = nlohmann::json;

string format_dir_name(string dir_name)
{
    int len = dir_name.length();
    if(dir_name == "/")
        return "/";
    
    if(dir_name[len-1] == '/')
        return dir_name.substr(0, len-1);

    return dir_name;
}


int main()
{
    string log;
	json_t req = get_req();

    // log = "¿ªÊ¼";
    // add_log(log);

	json_t res;
	res["result"] = -1;
	string task_type = req["task_type"];
    string username = req["username"];
    string token = req["token"];

	if(task_type == "mkdir")
	{
        string dir_name = req["dir_name"];
        dir_name = format_dir_name(dir_name);
		res = mkdir(username, dir_name, token);
		send_json(res);
		return 0;
	}
    else if(task_type == "copy_file")
    {
        string filename = req["file_name"];
        string dir_name = req["dir_name"];
        dir_name = format_dir_name(dir_name);
        res = copy_file(username, filename, dir_name, token);
        send_json(res);
        return 0;
    }
    else if(task_type == "delete_file")
    {
        string filename = req["file_name"];
        res = delete_file(username, filename, token);
        send_json(res);
        return 0;
    }
    else if(task_type == "copy_dir")
    {
        string dir_name = req["dir_name"];
        string dst_dir_name = req["dst_dir_name"];
        dir_name = format_dir_name(dir_name);
        dst_dir_name = format_dir_name(dst_dir_name);
        res = copy_dir(username, dir_name, dst_dir_name, token);
        send_json(res);
        return 0;
    }
    else if(task_type == "delete_dir")
    {
        string dir_name = req["dir_name"];
        dir_name = format_dir_name(dir_name);
        res = delete_dir(username, dir_name, token);
        send_json(res);
        return 0;
    }
    else if(task_type == "cut_file")
    {
        string filename = req["file_name"];
        string dir_name = req["dir_name"];
        dir_name = format_dir_name(dir_name);
        res = cut_file(username, filename, dir_name, token);
        send_json(res);
        return 0;
    }
    else if(task_type == "cut_dir")
    {
        string dir_name = req["dir_name"];
        string dst_dir_name = req["dst_dir_name"];
        dir_name = format_dir_name(dir_name);
        dst_dir_name = format_dir_name(dst_dir_name);
        res = cut_dir(username, dir_name, dst_dir_name, token);
        send_json(res);
        return 0;
    }
    else if(task_type == "file_rename")
    {
        string filename = req["file_name"];
        string newname = req["new_name"];
        res = file_rename(username, filename, newname, token);
        send_json(res);
        return 0;
    }

	return 0;
}