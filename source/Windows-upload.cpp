#include <tools.h>
#include <json.hpp>
#include <iostream>
using namespace std;
using json_t = nlohmann::json;

string get_value(string& req, string key)
{
        key = ' ' + key + '=';
        int pos = req.find(key);
        int len = key.length();
        int curpos = pos + len;
        if(req[curpos] == '\"')
        {
                len = 0;
                curpos += 1;
                int p = curpos;
                while(req[p] != '\"')
                {
                        len++;
                        p++;
                }
                return req.substr(curpos, len);
        }
        else
        {
                len = 0;
                int p = curpos;
                while(req[p] != ' ' && req[p] != '\n' && req[p] != '\r')
                {
                        len++;
                        p++;
                }
                return req.substr(curpos, len);
        }
        return "failed";

}

int find_file_begin(string& req)
{
        string begin_token = BEGIN_TOKEN;
        int len = begin_token.length();
        int pos = req.find(begin_token);
        pos += len;
        pos += 4;
        return pos;
}

long long int readbuff(char* buff, long long int size)
{
        long long int offset = 0;
        while(offset < size)
        {
                long long int n = read(0, buff + offset, TRANS_BUFF_SIZE);
                offset += n;
        }
        return offset;
}

int main()
{
        string log;
        json_t res;
        char *allsize_str = getenv("CONTENT_LENGTH");
        long long int allsize = atoi(allsize_str);
	char buff[TRANS_BUFF_SIZE];
	string req;
	long long int n = readbuff(buff, allsize);
        // log = "读入的大小n：" + to_string(n);
        // add_log(log);


        //string tmp(buff, TRANS_BUFF_SIZE);
        string tmp(buff, TRANS_BUFF_SIZE);
        //add_log(tmp);
        if(tmp.rfind(FILE_END) == string::npos)
        {
                log = "文件上传失败，缺失文件结尾";
                add_log(log);
                res["result"] = 2;
                send_json(res);
                return 0;
        }
        //string().swap(tmp);
        tmp = "";

	req = string(buff, TRANS_BUFF_SIZE);
	string offset, size, tid;
	offset = get_value(req, "offset");
	size = get_value(req, "size");
	tid = get_value(req, "tid");
	int fb = find_file_begin(req);
        req = "";

	json_t order;
	order["offset"] = atoi(offset.c_str());
	order["size"] = atoi(size.c_str());
	order["tid"] = atoi(tid.c_str());
	order["fb"] = fb;

	res = win_upload(order, buff);

	send_json(res);
	return 0;
}