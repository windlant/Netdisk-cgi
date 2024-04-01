#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <sys/types.h>
#include <unistd.h>
#include <json.hpp>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#define PIECE_SIZE 2048*1024
//#define PIECE_SIZE 2048 * 1024
#define LOG_FILE_DIR "/etc/1953186pan/log/logs"
#define REAL_FILE_DIR "/etc/1953186pan/files/"
#define CONFIG_DIR "/etc/1953186pan/config/mypan.conf"
#define BEGIN_TOKEN "Content-Type: application/octet-stream"
#define FILE_END "------WebKitFordsjluagkhuffdsgaskddhadksghfgvfhsjavdskadasmBoundary"
#define DOWNLOAD_DATA_HEAD "Content-type: application/octet-stream\r\n\r\n"
#define DOWNLOAD_DATA_BEGIN_TOKEN "Content-Type: application/octet-stream\r\n\r\n"
#define DOWNLOAD_DATA_END_TOKEN "\r\n\r\n------WebKitFordsjluagkhuffdsgaskddhadksghfgvfhsjavdskadasmBoundary"
#define SQL_BUFF_MAX 1024
#define TRANS_BUFF_SIZE 3072*1024
using namespace std;
using json_t = nlohmann::json;

//数据库
class MyDb
{
public:
    MyDb();
    ~MyDb();
    bool initDB();
    bool exeSQL(std::string sql, vector<vector<string>>& res, int& row_num, int& col_num);

private:
    MYSQL* mysql;
    MYSQL_RES* result;
    MYSQL_ROW row;
};


//日志写入
void add_log(string log);


struct Packet
{
    int dataLen;    // 包头，用来记录存数据的大小
    char buf[1000]; // 包数据，用来存放数据
};
//通用
//获取请求josn
json_t get_req();
//生成随机字符串
string generate_str(int str_len = 32);
//发送json
void send_json(json_t res);

//获取所有文件、目录
json_t get_all_files(string username, string token);

//创建目录
json_t mkdir(string username, string dir_name, string token);

//复制目录
json_t copy_dir(string username, string dir_name, string dst_dir_name, string token);

//删除目录
json_t delete_dir(string username, string dir_name, string token);

//剪切目录
json_t cut_dir(string username, string dir_name, string dst_dir_name, string token);

//复制文件
json_t copy_file(string username, string filename, string dir_name, string token);

//删除文件
json_t delete_file(string username, string filename, string token);

//剪切文件
json_t cut_file(string username, string filename, string dir_name, string token);

//文件重命名
json_t file_rename(string username, string filename, string newname, string token);

//获取任务信息
json_t get_all_task(string username, string token);

//数据库操作
//user
bool get_user_by_name(string name, vector<vector<string>>& res, int& row, int& col);
bool add_user(string name, string passwd, int root_did, string token = "default");


//dir
bool add_dir(string username, string name = "/", int f_did = -1);
bool get_dir_by_user_name(string name, vector<vector<string>>& res, int& row, int& col);



//登录注册
json_t win_register(string uname, string pwd);
json_t win_login(string uname, string pwd);

//上传任务建立
json_t win_create_file_up_task(json_t order);

//下载任务建立
json_t win_create_file_down_task(json_t order);

//文件上传
json_t win_upload(json_t& order, char* buff);


//文件下载
json_t win_download(json_t& order, char* buff);