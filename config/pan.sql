/*
SQLyog Community v13.1.6 (64 bit)
MySQL - 10.3.28-MariaDB : Database - pan
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
CREATE DATABASE /*!32312 IF NOT EXISTS*/`pan` /*!40100 DEFAULT CHARACTER SET gbk */;

USE `pan`;

/*Table structure for table `dir` */

DROP TABLE IF EXISTS `dir`;

CREATE TABLE `dir` (
  `did` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(500) NOT NULL,
  `f_did` int(11) NOT NULL DEFAULT -1,
  `username` varchar(50) NOT NULL,
  PRIMARY KEY (`did`)
) ENGINE=InnoDB AUTO_INCREMENT=922 DEFAULT CHARSET=gbk;

/*Data for the table `dir` */

/*Table structure for table `file` */

DROP TABLE IF EXISTS `file`;

CREATE TABLE `file` (
  `fid` int(11) NOT NULL AUTO_INCREMENT,
  `filesize` bigint(20) NOT NULL,
  `md5` varchar(32) NOT NULL,
  `link_num` int(11) NOT NULL DEFAULT 0,
  `name` varchar(500) NOT NULL,
  `type` varchar(20) NOT NULL,
  PRIMARY KEY (`fid`)
) ENGINE=InnoDB AUTO_INCREMENT=965 DEFAULT CHARSET=gbk;

/*Data for the table `file` */

/*Table structure for table `file_map` */

DROP TABLE IF EXISTS `file_map`;

CREATE TABLE `file_map` (
  `fmid` int(11) NOT NULL AUTO_INCREMENT,
  `fid` int(11) NOT NULL,
  `username` varchar(50) NOT NULL,
  `did` int(11) NOT NULL,
  `name` varchar(500) NOT NULL,
  `type` varchar(20) NOT NULL,
  `md5` varchar(32) NOT NULL,
  PRIMARY KEY (`fmid`)
) ENGINE=InnoDB AUTO_INCREMENT=1123 DEFAULT CHARSET=gbk;

/*Data for the table `file_map` */

/*Table structure for table `task` */

DROP TABLE IF EXISTS `task`;

CREATE TABLE `task` (
  `tid` int(11) NOT NULL AUTO_INCREMENT,
  `f_tid` int(11) NOT NULL DEFAULT -1,
  `type` varchar(10) NOT NULL,
  `offset` bigint(20) NOT NULL DEFAULT 0,
  `size` bigint(20) NOT NULL,
  `last_time` bigint(20) unsigned NOT NULL,
  `ftype` varchar(20) NOT NULL DEFAULT 'null',
  `dst_did` int(11) NOT NULL DEFAULT -1,
  `md5` varchar(32) NOT NULL,
  `username` varchar(50) NOT NULL,
  `filename` varchar(500) NOT NULL DEFAULT 'null',
  `realname` varchar(500) NOT NULL,
  `local_dir` varchar(500) NOT NULL DEFAULT '/',
  PRIMARY KEY (`tid`)
) ENGINE=InnoDB AUTO_INCREMENT=1410 DEFAULT CHARSET=gbk;

/*Data for the table `task` */

/*Table structure for table `user` */

DROP TABLE IF EXISTS `user`;

CREATE TABLE `user` (
  `uid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(50) NOT NULL,
  `passwd` varchar(32) NOT NULL,
  `token` varchar(32) DEFAULT NULL,
  `root_did` int(11) NOT NULL,
  PRIMARY KEY (`uid`)
) ENGINE=InnoDB AUTO_INCREMENT=14 DEFAULT CHARSET=gbk;

/*Data for the table `user` */

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
