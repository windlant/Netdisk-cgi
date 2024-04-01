#include <tools.h>
#include <json.hpp>
#include <iostream>
using namespace std;
using json_t = nlohmann::json;

void send_msg(string msg)
{
    cout<<"Content-type:text/plain\r\n\r\n";
    cout<<msg;
}

int main()
{
    string log;
    // log = "进入下载文件入口函数";
    // add_log(log);

	json_t req = get_req();
    json_t res;
	char buff[TRANS_BUFF_SIZE];
	res = win_download(req, buff);
	int ret = res["result"];
    if(ret == 1)
    {
        send_msg("文件下载出错，没有找到对应任务！");
        return 0;
    }
    else if(ret == 2)
    {
        send_msg("文件下载出错，读入的字节数与预期不符！");
        return 0;
    }
    else if(ret == 3)
    {
        send_msg("finished");
        return 0;
    }
    else if(ret == 0)
    {
        long long int buff_size = res["buff_size"];
        write(1, buff, buff_size);
        fflush(stdout);
        return 0;
    }
	return 0;
}