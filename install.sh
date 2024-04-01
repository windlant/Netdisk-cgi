cd source; make clean
make
mkdir -p /var/www/cgi-bin/src
mkdir -p /etc/1953186pan/log
mkdir -p /etc/1953186pan/files
mkdir -p /etc/1953186pan/config
touch /etc/1953186pan/log/logs


/bin/cp Windows-create-task.cgi /var/www/cgi-bin/src
/bin/cp Windows-download.cgi /var/www/cgi-bin/src
/bin/cp Windows-get-task.cgi /var/www/cgi-bin/src
/bin/cp Windows-getfiles.cgi /var/www/cgi-bin/src
/bin/cp Windows-login.cgi /var/www/cgi-bin/src
/bin/cp Windows-other.cgi /var/www/cgi-bin/src
/bin/cp Windows-register.cgi /var/www/cgi-bin/src
/bin/cp Windows-upload.cgi /var/www/cgi-bin/src

cd ../config
/bin/cp mypan.conf /etc/1953186pan/config
/bin/cp enable_apache.sh /etc/1953186pan/config

chmod a+w /var/www/cgi-bin/src
chmod a+w /etc/1953186pan/config
chmod a+w /etc/1953186pan/config/mypan.conf
chmod a+w /etc/1953186pan/files
chmod a+w /etc/1953186pan/log
chmod a+w /etc/1953186pan/log/logs
chmod a+w /var/www/cgi-bin/src/Windows-create-task.cgi 
chmod a+w /var/www/cgi-bin/src/Windows-download.cgi
chmod a+w /var/www/cgi-bin/src/Windows-get-task.cgi
chmod a+w /var/www/cgi-bin/src/Windows-getfiles.cgi
chmod a+w /var/www/cgi-bin/src/Windows-login.cgi
chmod a+w /var/www/cgi-bin/src/Windows-other.cgi
chmod a+w /var/www/cgi-bin/src/Windows-register.cgi
chmod a+w /var/www/cgi-bin/src/Windows-upload.cgi
systemctl restart httpd

echo 已完成安装
