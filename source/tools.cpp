#include <vector>
#include <iostream>
#include <tools.h>
#include <json.hpp>

using namespace std;
using json_t = nlohmann::json;

//去除空格
void trim(string &s)
{
	int index = 0;
	if(!s.empty())
	{
		while( (index = s.find(' ',index)) != string::npos)
		{
			s.erase(index,1);
		}
	}
}

//读取配置文件
static bool readConfigFile(const char * cfgfilepath, const string & key, string & value)
{
    fstream cfgFile;
    cfgFile.open(cfgfilepath);//打开文件	
    if( ! cfgFile.is_open())
    {
        cout<<"can not open cfg file!"<<endl;
        return false;
    }
    char tmp[1000];
    while(!cfgFile.eof())//循环读取配置文件每一行
    {
        cfgFile.getline(tmp,1000);
        string line(tmp);
		trim(line);
        size_t pos = line.find('=');
        if(pos==string::npos) return false;
        string tmpKey = line.substr(0,pos);
        if(key==tmpKey)
        {
            value = line.substr(pos+1);
            return true;
        }
    }
    return false;
}

//数据库
MyDb::MyDb()
{
	mysql = mysql_init(NULL);

	if (!mysql)
	{
		cout << "Error:" << mysql_error(mysql);
		exit(1);
	}
}

MyDb::~MyDb()
{
	if (mysql)
	{
		mysql_close(mysql);
	}
}


bool MyDb::initDB()
{
	string log;
	string host;
	string user;
	string passwd;
	string db_name;
	string port_str;

	readConfigFile(CONFIG_DIR, "host", host);
	readConfigFile(CONFIG_DIR, "user", user);
	readConfigFile(CONFIG_DIR, "passwd", passwd);
	readConfigFile(CONFIG_DIR, "db_name", db_name);
	readConfigFile(CONFIG_DIR, "port", port_str);

	int port = atoi(port_str.c_str());

	mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8");//设置字符集
	//mysql_set_character_set(mysql, "utf8"); 
	mysql = mysql_real_connect(mysql, host.c_str(), user.c_str(), passwd.c_str(), db_name.c_str(), port, NULL, 0);
	if (!mysql)
	{
		string mysql_error_str = string(mysql_error(mysql));
		log = "数据库初始化失败！" + mysql_error_str;
		add_log(log);
		return false;
	}

	return true;
}

bool MyDb::exeSQL(std::string sql, vector<vector<string>>& res, int& row_num, int& col_num)
{
	
	if (mysql_query(mysql, sql.c_str()))
	{
		string log;
		string mysql_error_str = string(mysql_error(mysql));
		log = "数据库执行sql失败，错误提示：" + mysql_error_str + " \nSQL语句：" + sql;
		add_log(log);
		return false;
	}

	result = mysql_store_result(mysql);

	if (result)
	{
		
		int num_fields = mysql_num_fields(result);
		unsigned long long num_rows = mysql_num_rows(result);
		row_num = num_rows;
		col_num = num_fields;
		res.clear();
		for (unsigned long long i = 0; i < num_rows; i++)
		{
			row = mysql_fetch_row(result);
			if (!row)
			{
				break;
			}

			vector<string>  tmp;
			for (int j = 0; j < num_fields; j++)
			{
				tmp.push_back(row[j]);
			}
			res.push_back(tmp);
		}
	}
	else
	{
		
		if (mysql_field_count(mysql) == 0)
		{
			
			row_num = mysql_affected_rows(mysql);
			return true;
		}
		else
		{
			row_num = 0;
			col_num = 0;
			return false;
		}
	}

	return true;
}


json_t get_all_task(string username, string token)
{
	string log;

	json_t res;
	res["result"] = -1;
	char sql_buf[SQL_BUFF_MAX];
	MyDb db;
	db.initDB();
	vector<vector<string>> sqlres;
	int row, col;
	string sql;

	sprintf(sql_buf, "select * from user where name = '%s'and token = '%s'", username.c_str(), token.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 1)
	{
		log = username + "获取任务列表， 用户认证失败！";
		add_log(log);
		res["result"] = 1;
		return res;
	}

	vector<json_t> up_tasks;
	vector<json_t> down_tasks;

	sprintf(sql_buf, "select tid,filename,offset,size,last_time,local_dir from task where username = '%s'and type = 'file_up'", username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	for(int i = 0; i < row; i++)
	{
		json_t tmp;
		string local_dir = sqlres[i][5];
		tmp["tid"] = atoi(sqlres[i][0].c_str());
		tmp["filename"] = sqlres[i][1];
		tmp["offset"] = atoi(sqlres[i][2].c_str());
		tmp["size"] = atoi(sqlres[i][3].c_str());
		tmp["last_time"] = atoi(sqlres[i][4].c_str());
		tmp["local_dir"] = local_dir;
		up_tasks.push_back(tmp);
	}

	sprintf(sql_buf, "select tid,size,filename,local_dir,last_time,offset from task where username = '%s'and type = 'file_down'", username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	for(int i = 0; i < row; i++)
	{
		json_t tmp;
		tmp["tid"] = atoi(sqlres[i][0].c_str());
		tmp["size"] = atoi(sqlres[i][1].c_str());
		tmp["filename"] = sqlres[i][2];
		tmp["local_dir"] = sqlres[i][3];
		tmp["last_time"] = atoi(sqlres[i][4].c_str());
		tmp["offset"] = atoi(sqlres[i][5].c_str());
		down_tasks.push_back(tmp);
	}

	log = username + "获取任务列表成功！";
	add_log(log);

	res["result"] = 0;
	res["up_tasks"] = up_tasks;
	res["down_tasks"] = down_tasks;
	return res;
}


//写入日志
void add_log(string log)
{
    char buf[300];
    time_t tt = time(NULL);
    tm* t= localtime(&tt);
    sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d ",
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec);
    ofstream out;
    string time1 = string(buf);
    log = time1 + ' ' + log + '\n';
    out.open(LOG_FILE_DIR, ios::app);
    out<<log;
    out.close();
}

//解析客户端请求为json格式
json_t get_req()
{
	string req;
	string res = "";
	while (cin >> req)
	{
		res = res + req;
	}
	json_t j;
	j = json_t::parse(res);
	return j;
}

//获取用户文件列表
json_t get_all_files(string username, string token)
{
	string log;
	json_t res;
	res["result"] = -1;
	char sql_buf[SQL_BUFF_MAX];
	MyDb db;
	db.initDB();
	vector<vector<string>> sqlres;
	int row, col;
	string sql;

	sprintf(sql_buf, "select * from user where name = '%s'and token = '%s'", username.c_str(), token.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 1)
	{
		log = username+"获取文件失败， 用户认证失败！";
		add_log(log);
		res["result"] = 1;
		return res;
	}

	sprintf(sql_buf, "select dir.name,file_map.name from file_map,dir where file_map.username = '%s' and file_map.did = dir.did", username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	vector<string> files;
	for(int i = 0; i<row; i++)
	{
		string file_name;
		string dir_name = sqlres[i][0];
		if(dir_name == "/")
			file_name = sqlres[i][0] + sqlres[i][1];
		else
			file_name = sqlres[i][0] + "/" + sqlres[i][1];
		files.push_back(file_name);
	}
	res["files"] = files;

	sprintf(sql_buf, "select name from dir where username = '%s'", username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	vector<string> dirs;
	for(int i = 0; i<row; i++)
	{
		dirs.push_back(sqlres[i][0]);
	}
	log = username + "获取文件列表成功";
	add_log(log);
	res["dirs"] = dirs;
	res["result"] = 0;
	return res;
}

string get_f_dir(string dir_name)
{
	if(dir_name == "/")
	{
		return "NULL";
	}

	int len = dir_name.length();
	int i = len - 1;
	while(dir_name[i] != '/')
		i--;
	if(i == 0)
		return "/";

	return dir_name.substr(0, i);
}

string get_raw_name(string dir_name)
{
	if(dir_name == "/")
	{
		return "/";
	}

	int len = dir_name.length();
	int i = len - 1;
	int count = 0;
	while(dir_name[i] != '/')
	{
		i--;
		count++;
	}

	return dir_name.substr(i+1, count);
}

json_t mkdir(string username, string dir_name, string token)
{
	string log;

	json_t res;
	res["result"] = -1;
	char sql_buf[SQL_BUFF_MAX];
	MyDb db;
	db.initDB();
	vector<vector<string>> sqlres;
	int row, col;
	string sql;

	sprintf(sql_buf, "select * from user where name = '%s'and token = '%s'", username.c_str(), token.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 1)
	{
		log = username+"创建目录失败， 用户认证失败！";
		add_log(log);
		res["result"] = 3;
		return res;
	}

	string f_dir_name = get_f_dir(dir_name);
	sprintf(sql_buf, "select * from dir where name = '%s' and username='%s'", dir_name.c_str(), username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if(row > 0)
	{
		log = username + "创建目录成功，已存在该目录，目录名：" + dir_name;
		add_log(log);
		res["result"] = 0;
		return res;
	}


	sprintf(sql_buf, "select * from dir where name = '%s' and username='%s'", f_dir_name.c_str(), username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if(row == 0)
	{
		log = username + "创建目录失败，父目录不存在，目录名：" + dir_name;
		add_log(log);
		res["result"] = 2;
		return res;
	}

	string f_did = sqlres[0][0];
	sprintf(sql_buf, "insert into dir (name, f_did, username) values('%s', %s, '%s')", dir_name.c_str(), f_did.c_str(), username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);


	// row = 0;
	// while(row == 0)
	// {
	// 	sleep(1);
	// 	sprintf(sql_buf, "select * from dir where name = '%s' and username='%s'", dir_name.c_str(), username.c_str());
	// 	sql = string(sql_buf);
	// 	db.exeSQL(sql, sqlres, row, col);
	// }

	log = username + "创建目录成功，目录名：" + dir_name;
	add_log(log);
	res["result"] = 0;

	return res;
}

//复制文件
json_t copy_file(string username,string filename, string dir_name, string token)
{
	string log;

	json_t res;
	res["result"] = -1;
	char sql_buf[SQL_BUFF_MAX];
	MyDb db;
	db.initDB();
	vector<vector<string>> sqlres;
	int row, col;
	string sql;

	string file_raw_name = get_raw_name(filename);

	sprintf(sql_buf, "select * from user where name = '%s'and token = '%s'", username.c_str(), token.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 1)
	{
		log = username+"复制失败， 用户认证失败！";
		add_log(log);
		res["result"] = 4;
		return res;
	}

	//检查目录
	sprintf(sql_buf, "select * from dir where name = '%s' and username='%s'", dir_name.c_str(), username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if(row != 1)
	{
		log = username + "复制文件失败，目标目录不存在" + " 目标目录：" + dir_name;
		add_log(log);
		res["result"] = 2;
		return res;
	}
	string tar_dir_id = sqlres[0][0];


	//检查同名文件
	sprintf(sql_buf, "select * from file_map where name = '%s' and did = %s and username = '%s'", file_raw_name.c_str(), tar_dir_id.c_str(), username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if(row != 0)
	{
		log = username + "复制文件取消，已存在同名文件" + " 文件名：" + filename + " 目标目录：" + dir_name;
		add_log(log);
		res["result"] = 0;
		return res;
	}
	string f_dir_name = get_f_dir(filename);


	//检查该文件
	sprintf(sql_buf, "select * from dir where name = '%s' and username='%s'", f_dir_name.c_str(), username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if(row != 1)
	{
		log = username + "复制文件失败，父目录不存在" + " 父目录：" + f_dir_name;
		add_log(log);
		res["result"] = 3;
		return res;
	}


	string f_dir_id = sqlres[0][0];


	sprintf(sql_buf, "select * from file_map where name = '%s' and did = %s and username = '%s'", file_raw_name.c_str(), f_dir_id.c_str(), username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);

	log = "行数" + to_string(row);
	add_log(log);

	if(row != 1)
	{
		log = username + "复制文件失败，不存在该文件" + " 文件名：" + filename;
		add_log(log);
		res["result"] = 3;
		return res;
	}


	string fid = sqlres[0][1];
	string md5 = sqlres[0][6];

	//开始复制
	sprintf(sql_buf, "insert into file_map (fid, username, did, name, type, md5) values(%s, '%s', %s, '%s', '%s', '%s')",
	fid.c_str(), username.c_str(), tar_dir_id.c_str(), file_raw_name.c_str(), "0", md5.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);


	sprintf(sql_buf, "update file set link_num = link_num + 1 where fid = %s",fid.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);

	log = username + "复制文件成功" + " 文件名：" + filename + " 目标目录：" + dir_name;
	add_log(log);
	res["result"] = 0;
	return res;
}

//删除文件
json_t delete_file(string username, string filename, string token)
{
	string log;

	json_t res;
	res["result"] = -1;
	char sql_buf[SQL_BUFF_MAX];
	MyDb db;
	db.initDB();
	vector<vector<string>> sqlres;
	int row, col;
	string sql;

	string file_raw_name = get_raw_name(filename);
	string f_dir_name = get_f_dir(filename);

	sprintf(sql_buf, "select * from user where name = '%s'and token = '%s'", username.c_str(), token.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 1)
	{
		log = username+"删除失败， 用户认证失败！";
		add_log(log);
		res["result"] = 2;
		return res;
	}

	sprintf(sql_buf, "select file_map.fmid, file_map.fid from file_map,dir where file_map.name = '%s' and file_map.username = '%s' and file_map.did = dir.did and dir.name = '%s'",
	file_raw_name.c_str(), username.c_str(), f_dir_name.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 1)
	{
		log = username+"删除失败， 不存在该文件！";
		add_log(log);
		res["result"] = 1;
		return res;
	}
	string fmid = sqlres[0][0];
	string fid = sqlres[0][1];

	//开始删除
	sprintf(sql_buf, "delete from file_map where fmid = %s", fmid.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);

	
	sprintf(sql_buf, "update file set link_num = link_num - 1 where fid = %s",fid.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);

	sprintf(sql_buf, "select link_num, name from file where fid = %s", fid.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	int link_num = atoi(sqlres[0][0].c_str());
	string realname = sqlres[0][1];
	if(link_num == 0)
	{
		sprintf(sql_buf, "delete from file where fid = %s", fid.c_str());
		sql = string(sql_buf);
		db.exeSQL(sql, sqlres, row, col);

		realname = REAL_FILE_DIR + realname;
		if(remove(realname.c_str()) == -1)
		{
			log = username+"文件删除失败， 真实文件删除失败" + " 文件名：" + filename;
			add_log(log);
			res["result"] = 3;
		}
	}

	log = username+"文件删除成功" + " 文件名：" + filename;
	add_log(log);
	res["result"] = 0;
	return res;
}

//判断dir2是否是dir1的子目录
int judge_dir_relation(string dir1, string dir2)
{
	int len1 = dir1.length();
	int len2 = dir2.length();

	if(len2 < len1)
		return 0;
	
	for(int i = 0; i<len1; i++)
	{
		if(dir1[i] != dir2[i])
			return 0;
	}

	return 1;
}

//复制目录
json_t copy_dir(string username, string dir_name, string dst_dir_name, string token)
{
	string log;
	json_t res;
	res["result"] = -1;
	char sql_buf[SQL_BUFF_MAX];
	MyDb db;
	db.initDB();
	vector<vector<string>> sqlres;
	int row, col;
	string sql;

	if(dir_name == "/")
	{
		log = username+"目录复制失败， 不能复制根目录！";
		add_log(log);
		res["result"] = 3;
		return res;
	}

	int rt1 = judge_dir_relation(dir_name, dst_dir_name);
	if(rt1 == 1)
	{
		log = username + "目录复制失败，不能复制到子目录下， 目录名："  + dir_name + " 目标目录名：" + dst_dir_name;
		res["result"] = 5;
		return res;
	}

	//用户认证
	sprintf(sql_buf, "select * from user where name = '%s'and token = '%s'", username.c_str(), token.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 1)
	{
		log = username+"目录复制失败， 用户认证失败！";
		add_log(log);
		res["result"] = 4;
		return res;
	}

	//检查目标目录存在性
	sprintf(sql_buf, "select did from dir where name = '%s' and username='%s'", dst_dir_name.c_str(), username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if(row != 1)
	{
		log = username + "目录复制失败，目标目录不存在" + "  目标目录：" + dst_dir_name;
		add_log(log);
		res["result"] = 2;
		return res;
	}

	//获取本目录信息
	sprintf(sql_buf, "select did from dir where name = '%s' and username='%s'", dir_name.c_str(), username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if(row != 1)
	{
		log = username + "目录复制失败，要复制的目录不存在" + "  要复制的目录名：" + dir_name;
		add_log(log);
		res["result"] = 1;
		return res;
	}

	string did = sqlres[0][0];

	//创建目标位置目录
	string dir_raw_name = get_raw_name(dir_name);
	string tar_dir_name;
	if(dst_dir_name == "/")
	 	tar_dir_name = dst_dir_name + dir_raw_name;
	else
		tar_dir_name = dst_dir_name + "/" + dir_raw_name;
	json_t rt;
	rt = mkdir(username, tar_dir_name, token);
	int rt_num = rt["result"];
	if(rt_num != 0)
	{
		log = username + "目录复制失败，创建目录失败" + " 创建目录名：" + tar_dir_name;
		add_log(log);
		res["result"] = 3;
		return res;
	}

	//复制目录下所有文件和目录
	sprintf(sql_buf, "select name from file_map where did = %s and username='%s'", did.c_str(), username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	for(int i = 0; i < row; i++)
	{
		string file_name = sqlres[i][0];
		file_name = dir_name + "/" + file_name;
		rt = copy_file(username, file_name, tar_dir_name, token);
		rt_num = rt["result"];
		if(rt_num != 0)
		{
			log = username + "目录复制失败，文件复制失败" + " 文件名：" + file_name;
			add_log(log);
			res["result"] = 3;
			return res;
		}
	}

	sprintf(sql_buf, "select name from dir where f_did = %s and username='%s'", did.c_str(), username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	for(int i = 0; i < row; i++)
	{
		string tmp_dir_name = sqlres[i][0];
		rt = copy_dir(username, tmp_dir_name, tar_dir_name, token);
		rt_num = rt["result"];
		if(rt_num != 0)
		{
			log = username + "目录复制失败，子目录复制失败" + " 子目录名：" + tmp_dir_name;
			add_log(log);
			res["result"] = 3;
			return res;
		}
	}


	log = username + "目录复制成功， 要复制的目录名：" + dir_name + " 目标目录名：" + dst_dir_name;
	add_log(log);
	res["result"] = 0;
	return res;
}

//目录删除
json_t delete_dir(string username, string dir_name, string token)
{
	string log;
	json_t res;
	res["result"] = -1;
	char sql_buf[SQL_BUFF_MAX];
	MyDb db;
	db.initDB();
	vector<vector<string>> sqlres;
	int row, col;
	string sql;

	if(dir_name == "/")
	{
		log = username+"目录删除失败， 不能删除根目录！";
		add_log(log);
		res["result"] = 2;
		return res;
	}

	//用户认证
	sprintf(sql_buf, "select * from user where name = '%s'and token = '%s'", username.c_str(), token.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 1)
	{
		log = username+"目录删除失败， 用户认证失败！";
		add_log(log);
		res["result"] = 3;
		return res;
	}

	//获取本目录信息
	sprintf(sql_buf, "select did from dir where name = '%s' and username='%s'", dir_name.c_str(), username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if(row != 1)
	{
		log = username + "目录删除失败，要删除的目录不存在" + "  目录名：" + dir_name;
		add_log(log);
		res["result"] = 1;
		return res;
	}
	string did = sqlres[0][0];

	//删除目录下所有文件和目录
	json_t rt;
	int rt_num;
	sprintf(sql_buf, "select name from file_map where did = %s and username='%s'", did.c_str(), username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	for(int i = 0; i < row; i++)
	{
		string file_name = sqlres[i][0];
		file_name = dir_name + "/" + file_name;
		rt = delete_file(username, file_name, token);
		rt_num = rt["result"];
		if(rt_num != 0)
		{
			log = username + "目录删除失败，文件删除失败" + " 文件名：" + file_name;
			add_log(log);
			res["result"] = 2;
			return res;
		}
	}

	sprintf(sql_buf, "select name from dir where f_did = %s and username='%s'", did.c_str(), username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	for(int i = 0; i < row; i++)
	{
		string tmp_dir_name = sqlres[i][0];
		rt = delete_dir(username, tmp_dir_name, token);
		rt_num = rt["result"];
		if(rt_num != 0)
		{
			log = username + "目录删除失败，子目录删除失败" + " 子目录名：" + tmp_dir_name;
			add_log(log);
			res["result"] = 2;
			return res;
		}
	}

	sprintf(sql_buf, "delete from dir where did = %s", did.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);

	log = username + "目录删除成功， 删除的目录名：" + dir_name;
	add_log(log);
	res["result"] = 0;
	return res;
}

//剪切文件
json_t cut_file(string username ,string filename, string dir_name, string token)
{
	json_t res;
	string log;
	res = copy_file(username, filename, dir_name, token);
	int rt = res["result"];
	if(rt != 0)
	{
		log = username + "文件剪切失败，复制时失败 文件名：" + filename + " 目标目录：" + dir_name;
		add_log(log);
		return res;
	}

	res = delete_file(username, filename, token);
	rt = res["result"];
	if(rt == 0)
	{
		log = username + "文件剪切成功， 文件名：" + filename + " 目标目录：" + dir_name;
		add_log(log);
	}
	else
	{
		log = username + "文件剪切失败，删除时失败 文件名：" + filename + " 目标目录：" + dir_name;
		add_log(log);
	}

	return res;
}



//剪切目录
json_t cut_dir(string username, string dir_name, string dst_dir_name, string token)
{
	json_t res;
	string log;

	int rt = judge_dir_relation(dir_name, dst_dir_name);
	if(rt == 1)
	{
		log = username + "目录剪切失败，不能剪切到子目录下， 目录名："  + dir_name + " 目标目录名：" + dst_dir_name;
		res["result"] = 5;
		return res;
	}

	res = copy_dir(username, dir_name, dst_dir_name, token);
	rt = res["result"];
	if(rt != 0)
	{
		log = username + "目录剪切失败，复制时失败， 目录名："  + dir_name + " 目标目录名：" + dst_dir_name;
		add_log(log);
		return res;
	}

	res = delete_dir(username, dir_name, token);
	rt = res["result"];
	if(rt == 0)
	{
		log = username + "目录剪切成功， 目录名："  + dir_name + " 目标目录名：" + dst_dir_name;
		add_log(log);
	}
	else
	{
		log = username + "目录剪切失败，删除时失败， 目录名："  + dir_name + " 目标目录名：" + dst_dir_name;
		add_log(log);
	}

	return res;
}

//文件重命名
json_t file_rename(string username ,string filename, string newname, string token)
{
	string log;
	json_t res;
	res["result"] = -1;
	char sql_buf[SQL_BUFF_MAX];
	MyDb db;
	db.initDB();
	vector<vector<string>> sqlres;
	int row, col;
	string sql;

	sprintf(sql_buf, "select * from user where name = '%s'and token = '%s'", username.c_str(), token.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 1)
	{
		log = username+"文件重命名失败， 用户认证失败" + " 文件名：" + filename;
		add_log(log);
		res["result"] = 3;
		return res;
	}

	string file_raw_name = get_raw_name(filename);
	string f_dir_name = get_f_dir(filename);
	sprintf(sql_buf, "select file_map.fmid, file_map.fid from file_map,dir where file_map.name = '%s' and file_map.username = '%s' and file_map.did = dir.did and dir.name = '%s'",
	file_raw_name.c_str(), username.c_str(), f_dir_name.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 1)
	{
		log = username+"文件重命名失败， 不存在该文件" + " 文件名：" + filename;
		add_log(log);
		res["result"] = 1;
		return res;
	}

	string fmid = sqlres[0][0];

	sprintf(sql_buf, "select file_map.fmid, file_map.fid from file_map,dir where file_map.name = '%s' and file_map.username = '%s' and file_map.did = dir.did and dir.name = '%s'",
	newname.c_str(), username.c_str(), f_dir_name.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 0)
	{
		log = username+"文件重命名失败， 已存在同名文件" + " 目标文件名：" + newname;
		add_log(log);
		res["result"] = 2;
		return res;
	}

	sprintf(sql_buf, "update file_map set name = '%s' where fmid = %s",newname.c_str(), fmid.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	
	log = username+"文件重命名成功" + " 原文件：" + filename + " 目标文件名：" + newname;
	add_log(log);
	res["result"] = 0;
	return res;
}

//生成随机字符串
string generate_str(int str_len)
{
	string str;
	int i, flag;
    struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv,&tz);
	long long int micrsec = tv.tv_sec*1000000 + tv.tv_usec;
	srand(micrsec); 
	for (i = 0; i < str_len; i++)
	{
		flag = rand() % 3;
		switch (flag)
		{
		case 0:
			str += rand() % 26 + 'a';
			break;
		case 1:
			str += rand() % 26 + 'A';
			break;
		case 2:
			str += rand() % 10 + '0';
			break;
		}
	}
	return str;
}

//发送相应json结构体
void send_json(json_t res)
{
	long long int len = res.dump().length();
	string head_len = "Content-Length: " + to_string(len) + "\r\n";
	//cout<< "Connection: keep-alive\r\n";
	//cout<< "Connection: close\r\n";
	cout<< head_len;
	cout << "Content-Type: application/json\r\n\r\n";
	//cout << "Content-Type: text/plain\r\n\r\n";
	cout << res.dump();
	return;
}


//user
bool get_user_by_name(string name, vector<vector<string>>& res, int& row, int& col)
{
	MyDb db;
	db.initDB();
	string sql = "select * from user where name = '" + name + "'";
	if (!db.exeSQL(sql, res, row, col))
	{
		return false;
	}
	return true;
}

bool add_user(string name, string passwd, int root_did, string token)
{
	MyDb db;
	db.initDB();
	string sql = "insert into user (name, passwd, token, root_did) values('" + name + "','" + passwd + "','" + token + "', " + to_string(root_did) + ")";
	vector<vector<string>> sqlres;
	int row, col;
	if (!db.exeSQL(sql, sqlres, row, col))
	{
		return false;
	}
	return true;
}


//dir
bool add_dir(string username, string name, int f_did)
{
	MyDb db;
	db.initDB();
	string sql = "insert into dir (name, username, f_did) values('" + name + "','" + username + "'," + to_string(f_did) + ")";
	vector<vector<string>> sqlres;
	int row, col;
	if (!db.exeSQL(sql, sqlres, row, col))
	{
		return false;
	}
	return true;
}

bool get_dir_by_user_name(string username, vector<vector<string>>& res, int& row, int& col)
{
	MyDb db;
	db.initDB();
	string sql = "select * from dir where username = '" + username + "'";
	if (!db.exeSQL(sql, res, row, col))
	{
		return false;
	}
	return true;
}


json_t win_register(string uname, string pwd)
{
	string log;
	json_t res;
	vector<vector<string>> sqlres;
	int row, col;
	if (!get_user_by_name(uname, sqlres, row, col))
	{
		res["result"] = 1;
		res["msg"] = "check name failed";
		return res;
	}
	if (row > 0)
	{
		res["result"] = 1;
		res["msg"] = "name exists";
		return res;
	}
	
	if(!add_dir(uname))
	{
		res["result"] = 1;
		res["msg"] = "dir add failed";
		return res;
	}

	get_dir_by_user_name(uname, sqlres, row, col);
	int did = atoi(sqlres[0][0].c_str());

	if (!add_user(uname, pwd, did))
	{
		res["result"] = 1;
		res["msg"] = "dir add failed";
		return res;
	}

	res["result"] = 0;
	res["msg"] = "register suc";

	log = uname+"注册成功";
	add_log(log);
	return res;
}


json_t win_login(string uname, string pwd)
{
	string log;
	json_t res;
	res["result"] = -1;
	char sql_buf[SQL_BUFF_MAX];
	MyDb db;
	db.initDB();
	vector<vector<string>> sqlres;
	int row, col;
	string sql;

	sprintf(sql_buf, "select * from user where name = '%s'", uname.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 1)
	{
		res["result"] = 1;
		res["msg"] = "user not exist";
		log = uname+"登录失败、用户不存在";
		add_log(log);
		return res;
	}
	sprintf(sql_buf, "select * from user where name = '%s' and passwd = '%s'", uname.c_str(), pwd.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 1)
	{
		res["result"] = 2;
		res["msg"] = "password not correct";
		log = uname+"登录失败、密码错误";
		add_log(log);
		return res;
	}

	string token = generate_str();

	sprintf(sql_buf, "update user set token = '%s' where name = '%s'", token.c_str(), uname.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);

	res["result"] = 0;
	res["msg"] = "login suc";
	res["token"] = token;
	log = uname+"登录成功";
	add_log(log);
	return res;
}

//单文件上传任务创建
json_t win_create_file_up_task(json_t order)
{
	string log;

	json_t res;
	res["debug"] = "NULL";
	res["result"] = -1;
	char sql_buf[SQL_BUFF_MAX];
	MyDb db;
	db.initDB();
	vector<vector<string>> sqlres;
	int row, col;
	string sql;

	//用户认证
    string token = order["token"];
    string username = order["username"];
	sprintf(sql_buf, "select * from user where name = '%s'and token = '%s'", username.c_str(), token.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 1)
	{
		log = username+"创建上传文件任务失败，用户认证失败" + " token：" + token;
		add_log(log);
		res["result"] = 2;
		return res;
	}

	int tid = order["tid"];
	if(tid != -1)
	{
		sprintf(sql_buf, "select * from task where tid = %d", tid);
		sql = string(sql_buf);
		db.exeSQL(sql, sqlres, row, col);

		long long int offset2 = 0;
		long long int trans_size2 = -1;
		long long int size2 = atoi(sqlres[0][4].c_str());
		string filename2 = sqlres[0][10];
		offset2 = atoi(sqlres[0][3].c_str());
		res["result"] = 1;
		res["offset"] = offset2;
		if(size2 - offset2 > PIECE_SIZE)
			trans_size2 = PIECE_SIZE;
		else
			trans_size2 = size2 - offset2;
		res["size"] = trans_size2;
		res["tid"] = tid;
		log = username + "上传任务创建成功，已有任务"+ "  文件名：" + filename2;
		add_log(log);
		return res;
	}

	string filename = order["filename"];
	//检查目录
    string dst_dir_name = order["dst_dir"];
	sprintf(sql_buf, "select * from dir where name = '%s'and username = '%s'", dst_dir_name.c_str(), username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 1)
	{
		log = username+"创建上传文件任务失败，目标目录不存在"+ "  目录名：" + dst_dir_name;
		add_log(log);
		res["result"] = 3;
		return res;
	}


	string dst_did = sqlres[0][0];

	//检查同名
	string ftype = order["ftype"];
	string md5 = order["md5"];
	sprintf(sql_buf, "select * from file_map where name = '%s' and did = %s", filename.c_str(), dst_did.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row == 1)
	{
		string exist_md5 = sqlres[0][6];
		if (exist_md5 == md5)
		{
			log = username+"创建上传任务取消，已有相同文件（md5相同）"+ "  文件名：" + filename;
			add_log(log);
			res["result"] = 0;
			return res;
		}
		else
		{
			log = username+"创建上传任务取消，已有相同文件（md5不同）"+ "  文件名：" + filename;
			add_log(log);
			res["result"] = 4;
			return res;
		}
	}

	//检查秒传
	sprintf(sql_buf, "select * from file where md5 = '%s'", md5.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row == 1)// 秒传
	{
		string fid = sqlres[0][0];
		int link_num = atoi(sqlres[0][3].c_str());
		sprintf(sql_buf, "insert into file_map (fid, username, did, name, type, md5) values(%s, '%s', %s, '%s', '%s', '%s')",
		 fid.c_str(), username.c_str(), dst_did.c_str(), filename.c_str(), ftype.c_str(), md5.c_str());
		sql = string(sql_buf);
		db.exeSQL(sql, sqlres, row, col);
		link_num++;
		sprintf(sql_buf, "update file set link_num = %d where fid = %s", link_num, fid.c_str());
		sql = string(sql_buf);
		db.exeSQL(sql, sqlres, row, col);
		log = username+"上传任务创建成功，用户完成秒传"+ "  文件名：" + filename;
		add_log(log);
		res["result"] = 0;
		return res;
	}

	string ttype = order["type"];
	//int f_tid = order["f_tid"];
	int f_tid = -1;
	//检查任务是否已存在
	//这里没有检查上传中同名但是md5不一样的情况
	string local_dir = order["local_dir"];
	sprintf(sql_buf, "select * from task where type = '%s' and dst_did = %s and md5 = '%s' and filename = '%s' and username = '%s' and local_dir = '%s'"
	, ttype.c_str(), dst_did.c_str(), md5.c_str(), filename.c_str(), username.c_str(), local_dir.c_str());

	sql = string(sql_buf);

	db.exeSQL(sql, sqlres, row, col);

	long long int offset = 0;
	long long int trans_size = -1;
	long long int size = order["size"];
	if (row == 1)
	{
		offset = atoi(sqlres[0][3].c_str());
		int tid3 = atoi(sqlres[0][0].c_str());
		res["result"] = 1;
		res["offset"] = offset;
		if(size - offset > PIECE_SIZE)
			trans_size = PIECE_SIZE;
		else
			trans_size = size - offset;
		res["size"] = trans_size;
		res["tid"] = tid3;
		log = username+"上传任务创建成功，已有任务"+ "  文件名：" + filename;
		add_log(log);
		return res;
	}

	
	//新建任务
	unsigned long time1 = time(0);
	string realname = username + generate_str(20) + '_' + filename;


	sprintf(sql_buf, "insert into task (f_tid, type, offset, size, last_time, ftype, dst_did, md5, username, filename, realname, local_dir) values(%d, '%s', %lld, %lld, %lu, '%s', %s, '%s', '%s', '%s' ,'%s', '%s')",
	 f_tid, ttype.c_str(), offset, size, time1, ftype.c_str(), dst_did.c_str(), md5.c_str(), username.c_str(), filename.c_str(), realname.c_str(), local_dir.c_str());
	sql = string(sql_buf);


	db.exeSQL(sql, sqlres, row, col);


	res["result"] = 1;
	res["offset"] = 0;
	if(size - offset > PIECE_SIZE)
		trans_size = PIECE_SIZE;
	else
		trans_size = size - offset;
	res["size"] = trans_size;


	sprintf(sql_buf, "select * from task where type = '%s' and dst_did = %s and md5 = '%s' and filename = '%s' and username = '%s'"
	, ttype.c_str(), dst_did.c_str(), md5.c_str(), filename.c_str(), username.c_str());

	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 1)
	{
		log = username+"上传任务创建失败，任务数据库写入失败！"+ "  文件名：" + filename;
		add_log(log);
		res["result"] = 5;
		return res;
	}
	int tid2 = atoi(sqlres[0][0].c_str());
	res["tid"] = tid2;
			
	log = username+"上传任务创建成功，完成创建"+ "  文件名：" + filename;
	add_log(log);
	return res;
}

//文件上传
json_t win_upload(json_t& order,char* buff)
{
	string log;
	json_t res;
	res["debug"] = "NULL";
	res["result"] = -1;
	char sql_buf[SQL_BUFF_MAX];
	MyDb db;
	db.initDB();
	vector<vector<string>> sqlres;
	int row, col;
	string sql;

	json_t task;
	int tid = order["tid"];

	sprintf(sql_buf, "select * from task where tid = %d", tid);
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if(row != 1)
	{
		log = "上行传输文件失败，任务信息获取失败";
		add_log(log);
		res["result"] = 2;
		return res;
	}

	string t_f_tid = sqlres[0][1];
	string t_type = sqlres[0][2];
	string t_offset = sqlres[0][3];
	string t_size = sqlres[0][4];
	string t_last_time = sqlres[0][5];
	string t_ftype = sqlres[0][6];
	string t_dst_did = sqlres[0][7];
	string t_md5 = sqlres[0][8];
	string t_username = sqlres[0][9];
	string t_filename = sqlres[0][10];
	string t_realname = sqlres[0][11];

	string realname = t_realname;
	realname = REAL_FILE_DIR + realname;
	//开始写文件
	long long int offset = order["offset"];
	long long int trans_size = order["size"];
	long long int file_size  = atoi(t_size.c_str());
	if(offset != atoi(t_offset.c_str()))
	{
		log = t_username+"上行传输文件失败，字节偏移不匹配"+ "  文件名：" + t_filename;
		add_log(log);
		res["result"] = 2;
		return res;
	}

	//写入
	int fb = order["fb"];
	int fd = open(realname.c_str(), O_WRONLY|O_CREAT, 0666);
	lseek(fd, offset, SEEK_SET);
	long long int write_size = write(fd, buff + fb, trans_size);
	close(fd);

	if(write_size != trans_size)
	{
		log = t_username + "上行传输文件出错，写入字节数和要求字节数不等" + " 任务id：" + to_string(tid);
		add_log(log);
		res["result"] = 2;
		return res;
	}

	offset += trans_size;
	if(offset > file_size)
	{
		log = t_username+"上行传输文件失败，偏移大于总字节，请检查已上传的文件"+ "  文件名：" + t_filename;
		add_log(log);
		res["result"] = 2;
		return res;
	}
	//文件上传完成
	if(offset == file_size)
	{
		int new_file_size = file_size;
		string new_file_md5 = t_md5;
		string new_file_name = t_realname;
		string new_file_type = t_ftype;
		sprintf(sql_buf, "insert into file (filesize, md5, link_num, name, type) values(%d, '%s', %d, '%s', '%s')",
		new_file_size, new_file_md5.c_str(), 1, new_file_name.c_str(), new_file_type.c_str());
		sql = string(sql_buf);
		db.exeSQL(sql, sqlres, row, col);

		sprintf(sql_buf, "select * from file where md5 = '%s'", new_file_md5.c_str()),
		sql = string(sql_buf);
		db.exeSQL(sql, sqlres, row, col);
		int new_file_id = atoi(sqlres[0][0].c_str());

		sprintf(sql_buf, "insert into file_map (fid, username, did, name, type, md5) values(%d, '%s', %s, '%s', '%s', '%s')",
		 new_file_id, t_username.c_str(), t_dst_did.c_str(), t_filename.c_str(), t_ftype.c_str(), t_md5.c_str());
		sql = string(sql_buf);
		db.exeSQL(sql, sqlres, row, col);
		
		sprintf(sql_buf, "delete from task where tid = %d", tid);
		sql = string(sql_buf);
		db.exeSQL(sql, sqlres, row, col);

		res["result"] = 0;
		res["tid"] = tid;
		log = t_username+"上行传输文件成功，所有字节已接收"+ "  文件名：" + t_filename + " 字节数：" + to_string(offset);
		add_log(log);
		return res;
	}

	//更新上传任务信息
	unsigned long time1 = time(0);
	sprintf(sql_buf, "update task set offset=%lld, last_time = %lu where tid = %d", offset, time1, tid);
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);

	if(file_size - offset > PIECE_SIZE)
		trans_size = PIECE_SIZE;
	else
		trans_size = file_size - offset;
	res["result"] = 1;
	res["offset"] = offset;
	res["size"] = trans_size;
	res["tid"] = tid;
	log = t_username+"上行传输文件成功，已接收部分内容"+ "  文件名：" + t_filename + " 字节数：" + to_string(offset);
	add_log(log);
	return res;
}


//下载任务建立
json_t win_create_file_down_task(json_t order)
{
	string log;
	// log = "进入创建下载任务函数";
	// add_log(log);

	json_t res;
	res["debug"] = "NULL";
	res["result"] = -1;
	char sql_buf[SQL_BUFF_MAX];
	MyDb db;
	db.initDB();
	vector<vector<string>> sqlres;
	int row, col;
	string sql;


	string username = order["username"];
	string token = order["token"];


	sprintf(sql_buf, "select * from user where name = '%s'and token = '%s'", username.c_str(), token.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if (row != 1)
	{
		log = username+"下载任务创建失败， 用户认证失败！";
		add_log(log);
		res["result"] = 1;
		return res;
	}

	int tid = order["tid"];
	if(tid != -1)
	{
		sprintf(sql_buf, "select tid,size,md5,filename from task where tid = %d", tid);
		sql = string(sql_buf);
		db.exeSQL(sql, sqlres, row, col);

		string filesize_str2 = sqlres[0][1];
		long long int filesize2 = atoi(filesize_str2.c_str());
		string md52 = sqlres[0][2];
		string filename2 = sqlres[0][3];
		res["result"] = 0;
		res["tid"] = tid;
		res["filesize"] = filesize2;
		res["md5"] = md52;
		log = username + "文件下载任务创建成功，已存在相同任务" + " 文件名：" + filename2;
		add_log(log);
		return res;
	}
	// log = "完成用户认证";
	// add_log(log);
	string filename = order["filename"];
	string task_type = order["type"];
	string local_dir = order["local_dir"];
	string file_raw_name = get_raw_name(filename);
	string dir_name = get_f_dir(filename);


	sprintf(sql_buf, "select fid  from file_map,dir where file_map.name='%s' and file_map.did=dir.did and dir.name='%s' and file_map.username = '%s'"
	, file_raw_name.c_str(), dir_name.c_str(), username.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if(row != 1)
	{
		log = username + "文件下载任务创建失败，要下载的文件不存在" + " 文件名：" + filename;
		add_log(log);
		res["result"] = 2;
		return res;
	}

	// log = "完成文件检查";
	// add_log(log);

	//查询真实文件
	string fid = sqlres[0][0];
	sprintf(sql_buf, "select filesize,md5,name from file where fid = %s", fid.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	string filesize_str = sqlres[0][0];
	string md5 = sqlres[0][1];
	string realname = sqlres[0][2];


	// log = "完成真实文件搜索";
	// add_log(log);

	long long int filesize = atoi(filesize_str.c_str());

	//检查有无已存在的下载任务
	sprintf(sql_buf, "select tid,size,md5 from task where username='%s' and local_dir='%s' and md5='%s'"
	, username.c_str(), local_dir.c_str(), md5.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if(row == 1) //已存在任务
	{
		string tid_str = sqlres[0][0];
		filesize_str = sqlres[0][1];
		filesize = atoi(filesize_str.c_str());
		md5 = sqlres[0][2];
		int tid3 = atoi(tid_str.c_str());

		res["result"] = 0;
		res["tid"] = tid3;
		res["filesize"] = filesize;
		res["md5"] = md5;
		log = username + "文件下载任务创建成功，已存在相同任务" + " 文件名：" + filename;
		add_log(log);
		return res;
	}

	// log = "完成检查有无下载任务";
	// add_log(log);

	//新建任务
	unsigned long time1 = time(0);
	sprintf(sql_buf, "insert into task (type, size, last_time, md5, username, realname, local_dir, filename) values('%s', %lld, %lu, '%s', '%s', '%s', '%s', '%s')",
	 task_type.c_str(), filesize, time1, md5.c_str(), username.c_str(), realname.c_str(), local_dir.c_str(), filename.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);

	sprintf(sql_buf, "select tid from task where username='%s' and local_dir='%s' and md5='%s'"
	, username.c_str(), local_dir.c_str(), md5.c_str());
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	int tid2 = atoi(sqlres[0][0].c_str());
	res["result"] = 0;
	res["tid"] = tid2;
	res["filesize"] = filesize;
	res["md5"] = md5;
	log = username + "文件下载任务创建成功" + " 文件名：" + filename;
	add_log(log);
	return res;
}


//文件下载
json_t win_download(json_t& order, char* buff)
{
	string log;
	// log = "进入文件下载函数";
	// add_log(log);


	json_t res;
	res["debug"] = "NULL";
	res["result"] = -1;
	char sql_buf[SQL_BUFF_MAX];
	MyDb db;
	db.initDB();
	vector<vector<string>> sqlres;
	int row, col;
	string sql;

	int tid = order["tid"];
	int finish = order["finish"];
	if(finish == 1)
	{
		sprintf(sql_buf, "delete from task where tid = %d", tid);
		sql = string(sql_buf);
		db.exeSQL(sql, sqlres, row, col);
		res["result"] = 3;
		log = "文件完成下载，所有字节已传输";
		add_log(log);
		return res;
	}

	long long int offset = order["offset"];
	long long int size = order["size"];
	string username = order["username"];
	//检测任务是否已经完成


	// log = "tid=" + to_string(tid);
	// add_log(log);
	// log = "offset=" + to_string(offset);
	// add_log(log);
	// log = "size=" + to_string(size);
	// add_log(log);
	// log = "finish=" + to_string(finish);
	// add_log(log);


	// log = "完成任务是否完成检测";
	// add_log(log);

	//检查任务存在情况
	sprintf(sql_buf, "select size,md5,realname from task where tid=%d", tid);
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);
	if(row != 1)
	{
		log = username+"文件下载传输出错，未找到对应任务" + " 任务id：" + to_string(tid);
		add_log(log);
		res["result"] = 1;
		return res;
	}

	// log = "完成任务存在情况检测";
	// add_log(log);

	string filesize_str = sqlres[0][0];
	string md5 = sqlres[0][1];
	string realname = sqlres[0][2];

	//构造数据包
	char tmpbuf[1024] = {0};
	sprintf(tmpbuf, " tid=%d  offset=%lld  size=%lld \r\n\r\n", tid, offset, size);
	string tmpbuf_str = string(tmpbuf);
	memset(tmpbuf, 0, 1024);
	//计算content-length
	long long int content_size = tmpbuf_str.length() + strlen(DOWNLOAD_DATA_BEGIN_TOKEN) + size + strlen(DOWNLOAD_DATA_END_TOKEN);
	string content_length = "Content-Length: " + to_string(content_size) + "\r\n";

	string head =content_length  + DOWNLOAD_DATA_HEAD + tmpbuf_str + DOWNLOAD_DATA_BEGIN_TOKEN;
	int head_len = head.length();
	long long int buff_point = 0;
	memcpy(buff + buff_point, head.c_str(), head_len);
	buff_point += head_len;

	// log = "完成头部报文构造";
	// add_log(log);

	//开始读取数据
	realname = REAL_FILE_DIR + realname;
	int fd = open(realname.c_str(), O_RDONLY);
	lseek(fd, offset, SEEK_SET);
	long long int read_size = read(fd, buff + buff_point, size);
	if(read_size != size)
	{
		log = username + "文件下载传输出错，读取到的字节与要读的字节数不等" + " 任务id：" + to_string(tid) + " 读到的字节数：" 
		+ to_string(read_size) + "要求的字节数：" + to_string(size);
		add_log(log);
		res["result"] = 2;
		return res;
	}

	// log = "完成文件读取";
	// add_log(log);

	buff_point += size;
	string data_end = DOWNLOAD_DATA_END_TOKEN;
	memcpy(buff + buff_point, data_end.c_str(), data_end.length());
	buff_point += data_end.length();

	//更新下载任务信息
	unsigned long time1 = time(0);
	sprintf(sql_buf, "update task set offset=%lld, last_time = %lu where tid = %d", offset + size, time1, tid);
	sql = string(sql_buf);
	db.exeSQL(sql, sqlres, row, col);

	// log = "完成任务信息更新";
	// add_log(log);

	res["result"] = 0;
	res["buff_size"] = buff_point;
	log = username + "文件下载传输成功，本次传输字节数：" + to_string(size) + " 任务id："+ to_string(tid);
	add_log(log);
	return res;
}