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

//���ݿ�
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


//��־д��
void add_log(string log);


struct Packet
{
    int dataLen;    // ��ͷ��������¼�����ݵĴ�С
    char buf[1000]; // �����ݣ������������
};
//ͨ��
//��ȡ����josn
json_t get_req();
//��������ַ���
string generate_str(int str_len = 32);
//����json
void send_json(json_t res);

//��ȡ�����ļ���Ŀ¼
json_t get_all_files(string username, string token);

//����Ŀ¼
json_t mkdir(string username, string dir_name, string token);

//����Ŀ¼
json_t copy_dir(string username, string dir_name, string dst_dir_name, string token);

//ɾ��Ŀ¼
json_t delete_dir(string username, string dir_name, string token);

//����Ŀ¼
json_t cut_dir(string username, string dir_name, string dst_dir_name, string token);

//�����ļ�
json_t copy_file(string username, string filename, string dir_name, string token);

//ɾ���ļ�
json_t delete_file(string username, string filename, string token);

//�����ļ�
json_t cut_file(string username, string filename, string dir_name, string token);

//�ļ�������
json_t file_rename(string username, string filename, string newname, string token);

//��ȡ������Ϣ
json_t get_all_task(string username, string token);

//���ݿ����
//user
bool get_user_by_name(string name, vector<vector<string>>& res, int& row, int& col);
bool add_user(string name, string passwd, int root_did, string token = "default");


//dir
bool add_dir(string username, string name = "/", int f_did = -1);
bool get_dir_by_user_name(string name, vector<vector<string>>& res, int& row, int& col);



//��¼ע��
json_t win_register(string uname, string pwd);
json_t win_login(string uname, string pwd);

//�ϴ�������
json_t win_create_file_up_task(json_t order);

//����������
json_t win_create_file_down_task(json_t order);

//�ļ��ϴ�
json_t win_upload(json_t& order, char* buff);


//�ļ�����
json_t win_download(json_t& order, char* buff);