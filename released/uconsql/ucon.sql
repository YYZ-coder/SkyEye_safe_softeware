/*
SQLyog Ultimate - MySQL GUI v8.2 
MySQL - 5.5.27 : Database - ucon
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
CREATE DATABASE /*!32312 IF NOT EXISTS*/`ucon` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `ucon`;

/*Table structure for table `directory_control` */

CREATE TABLE `directory_control` (
  `directory_id` varchar(125) NOT NULL,
  `directory_name` varchar(45) NOT NULL,
  `directory_modify_rename` int(11) NOT NULL DEFAULT '1',
  `directory_modify_move` int(11) NOT NULL DEFAULT '1',
  `directory_modify_delete` int(11) NOT NULL DEFAULT '1',
  `directory_modify_newfile` int(11) NOT NULL DEFAULT '1',
  `directory_access` int(11) NOT NULL DEFAULT '1',
  `directory_read` int(11) NOT NULL,
  `object_id` varchar(45) NOT NULL,
  `process_identify` varchar(45) NOT NULL,
  `process_perrmit` int(11) NOT NULL,
  PRIMARY KEY (`directory_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `directory_control` */

LOCK TABLES `directory_control` WRITE;

insert  into `directory_control`(`directory_id`,`directory_name`,`directory_modify_rename`,`directory_modify_move`,`directory_modify_delete`,`directory_modify_newfile`,`directory_access`,`directory_read`,`object_id`,`process_identify`,`process_perrmit`) values ('0','C:bb',0,1,1,0,0,0,'0','809',0),('22688440','C:vvv',0,1,1,0,0,0,'0','88',0),('25334432','C:abc',0,1,1,0,0,0,'0','8089',0),('C:UsersYueDesktopfff55b','C:UsersYueDesktopfff55b',1,0,1,1,1,1,'0','0',0),('C:yui','C:yui',0,1,1,0,0,0,'0','123',0),('C:\\test','test',1,0,0,1,1,0,'(1;2)','&process_name:4|process_id:6?',1),('C:\\Users\\Yue\\Desktop\\fff\\vhj','C:\\Users\\Yue\\Desktop\\fff\\vhj',1,1,1,1,0,0,'0','0',0);

UNLOCK TABLES;

/*Table structure for table `file` */

CREATE TABLE `file` (
  `File_Path` varchar(45) NOT NULL,
  `File_Name` varchar(45) NOT NULL,
  `Owner` int(11) NOT NULL,
  `Security_Level` int(11) NOT NULL,
  `Access_Times` int(10) unsigned zerofill DEFAULT NULL,
  `Refused_Times` int(10) unsigned zerofill DEFAULT NULL,
  `FileType` smallint(6) DEFAULT NULL,
  PRIMARY KEY (`File_Path`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='\n';

/*Data for the table `file` */

LOCK TABLES `file` WRITE;

insert  into `file`(`File_Path`,`File_Name`,`Owner`,`Security_Level`,`Access_Times`,`Refused_Times`,`FileType`) values ('c:	tt.txt','c:	tt.txt',1,5,0000000000,0000000000,0),('c:\\.carefile.txt','carefile.txt',2,5,0000000000,0000000000,1),('C:\\646+','646+',0,0,0000000000,0000000000,0),('c:\\78.txt','78.txt',1,1,0000000000,0000000000,NULL),('c:\\89.txt','89.txt',1,1,0000000000,0000000000,0),('C:\\abc','abc',0,0,0000000000,0000000000,0),('C:\\acc','acc',0,0,0000000000,0000000000,0),('C:\\bbb','bbb',0,0,0000000000,0000000000,0),('C:\\fds','fds',0,0,0000000000,0000000000,0),('c:\\first.txt','first.txt',2,5,0000000000,0000000000,1),('C:\\gf.txt','gf.txt',0,0,0000000000,0000000000,0),('C:\\hhh','hhh',0,0,0000000000,0000000000,0),('C:\\insert.txt','C:\\insert.txt',1,5,0000000000,0000000000,0),('C:\\opi','opi',0,0,0000000000,0000000000,0),('c:\\second.txt','second.txt',2,5,0000000000,0000000000,1),('c:\\test.txt','test.txt',0,2,0000000000,0000000000,NULL),('c:\\third.txt','third.txt',2,5,0000000000,0000000000,1),('c:\\third2.txt','third2.txt',2,5,0000000000,0000000000,1),('c:\\third3.txt','third3.txt',2,5,0000000000,0000000000,1),('c:\\ttt.txt','ttt.txt',1,5,0000000000,0000000000,0),('C:\\tttt','tttt',0,0,0000000000,0000000000,0),('c:\\tttt.txt','tttt.txt',1,5,0000000000,0000000000,0),('c:\\ttttg.txt','ttttg.txt',1,5,0000000000,0000000000,0),('c:\\ttttgu.txt','ttttgu.txt',1,5,0000000000,0000000000,0),('c:\\ttttgut.txt','ttttgut.txt',1,5,0000000000,0000000000,0),('c:\\ttttguti.txt','ttttguti.txt',1,5,0000000000,0000000000,0),('c:\\ttttgutiy.tx','ttttgutiy.tx',1,1,0000000000,0000000000,0),('c:\\ttttgutiy.txt','ttttgutiy.txt',1,5,0000000000,0000000000,0),('c:\\ttttgutiyui.txt','ttttgutiyui.txt',1,1,0000000000,0000000000,0),('C:\\Users\\Yue\\Desktop\\345.txt','345.txt',0,0,0000000000,0000000000,0),('C:\\Users\\Yue\\Desktop\\fff\\55','55',0,0,0000000000,0000000000,0),('C:\\Users\\Yue\\Desktop\\fff\\55\\bb','bb',3,0,0000000000,0000000000,0),('C:\\Users\\Yue\\Desktop\\fff\\ff','ff',0,0,0000000000,0000000000,0),('C:\\Users\\Yue\\Desktop\\fff\\nnn','nnn',0,0,0000000000,0000000000,0),('C:\\Users\\Yue\\Desktop\\fff\\vhj','vhj',3,0,0000000000,0000000000,0),('C:\\Users\\Yue\\Desktop\\fff\\vvv','vvv',0,0,0000000000,0000000000,0),('C:\\Users\\Yue\\Desktop\\fff\\vvv\\cc','cc',3,0,0000000000,0000000000,0),('C:\\uuu','uuu',0,0,0000000000,0000000000,0),('C:\\vvv','vvv',0,0,0000000000,0000000000,0),('C:\\Windows','Windows',1,1,0000000000,0000000000,0),('C:\\yui','yui',0,0,0000000000,0000000000,0),('c:\\你好.txt','你好.txt',1,2,0000000000,0000000000,1);

UNLOCK TABLES;

/*Table structure for table `file_control` */

CREATE TABLE `file_control` (
  `file_path` varchar(120) NOT NULL,
  `file_read` int(11) NOT NULL,
  `file_write` int(11) NOT NULL,
  `file_modify` int(11) NOT NULL,
  `file_all` int(11) NOT NULL,
  `file_lock` int(11) NOT NULL,
  `file_unlock` int(11) NOT NULL,
  `file_execute` int(11) NOT NULL,
  `file_copy` int(11) NOT NULL,
  `object_id` varchar(45) NOT NULL,
  `process_id` varchar(45) NOT NULL,
  `process_permit` int(11) NOT NULL,
  `file_modify_rename` int(11) NOT NULL DEFAULT '1',
  `file_modify_move` int(11) NOT NULL DEFAULT '1',
  `file_modify_delete` int(11) NOT NULL DEFAULT '1',
  PRIMARY KEY (`file_path`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `file_control` */

LOCK TABLES `file_control` WRITE;

insert  into `file_control`(`file_path`,`file_read`,`file_write`,`file_modify`,`file_all`,`file_lock`,`file_unlock`,`file_execute`,`file_copy`,`object_id`,`process_id`,`process_permit`,`file_modify_rename`,`file_modify_move`,`file_modify_delete`) values ('C:sn.txt',0,0,0,0,0,0,0,0,'0','547',1,0,0,0),('C:UsersYueDesktop345.txt',0,0,0,0,0,0,0,0,'0','890',1,0,1,1);

UNLOCK TABLES;

/*Table structure for table `jurisdiction` */

CREATE TABLE `jurisdiction` (
  `idJurisdiction` int(11) NOT NULL,
  `Jurisdiction_Type` varchar(45) NOT NULL,
  PRIMARY KEY (`idJurisdiction`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `jurisdiction` */

LOCK TABLES `jurisdiction` WRITE;

insert  into `jurisdiction`(`idJurisdiction`,`Jurisdiction_Type`) values (1,'FILE_READ'),(2,'FILE_WRITE'),(3,'FILE_EXECUTE'),(4,'FILE_COPY'),(5,'FILE_MODIFY'),(6,'FILE_ALL'),(7,'PROCESS_CREATE'),(8,'PROCESS_CLOSE'),(9,'PROCESS_TERMITED'),(10,'RECORD_WATCH'),(11,'NETWORK_FORBIDDEN'),(12,'NETWORD_ALLOW'),(13,'FILE_LOCK'),(14,'FILE_UNLOCK');

UNLOCK TABLES;

/*Table structure for table `nethead` */

CREATE TABLE `nethead` (
  `IP` varchar(20) DEFAULT NULL,
  `localPort` varchar(10) DEFAULT NULL,
  `remotePort` varchar(10) DEFAULT NULL,
  `url` varchar(20) DEFAULT NULL,
  `owner` int(20) DEFAULT NULL,
  `proto` int(10) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `nethead` */

LOCK TABLES `nethead` WRITE;

UNLOCK TABLES;

/*Table structure for table `operation` */

CREATE TABLE `operation` (
  `idOperation` int(11) NOT NULL,
  `Operation_Type` varchar(45) NOT NULL,
  PRIMARY KEY (`idOperation`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `operation` */

LOCK TABLES `operation` WRITE;

insert  into `operation`(`idOperation`,`Operation_Type`) values (1,'FILE_READ'),(2,'FILE_WRITE'),(3,'FILE_EXECUTE'),(4,'FILE_COPY'),(5,'FILE_MODIFY'),(6,'FILE_ALL'),(7,'PROCESS_CREATE'),(9,'PROCESS_NET'),(10,'RECORD_WATCH'),(11,'NETHEAD_OPEN'),(13,'FILE_LOCK'),(14,'FILE_UNLOCK');

UNLOCK TABLES;

/*Table structure for table `operation_jurisdiction` */

CREATE TABLE `operation_jurisdiction` (
  `Operation_ID` int(11) NOT NULL,
  `Jurisdiction_ID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `operation_jurisdiction` */

LOCK TABLES `operation_jurisdiction` WRITE;

insert  into `operation_jurisdiction`(`Operation_ID`,`Jurisdiction_ID`) values (1,1),(1,5),(1,6),(2,2),(2,5),(2,6),(3,5),(3,6),(5,13),(5,6),(6,6),(6,14),(5,14),(6,13),(7,3),(7,6),(8,4),(8,6);

UNLOCK TABLES;

/*Table structure for table `process` */

CREATE TABLE `process` (
  `Owner` int(10) DEFAULT NULL,
  `processName` varchar(100) DEFAULT NULL,
  `processPath` varchar(100) DEFAULT NULL,
  `forbidOpen` tinyint(1) DEFAULT NULL,
  `forbidNet` tinyint(1) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `process` */

LOCK TABLES `process` WRITE;

UNLOCK TABLES;

/*Table structure for table `processmonitor` */

CREATE TABLE `processmonitor` (
  `owner_id` int(10) DEFAULT NULL,
  `process_id` int(10) DEFAULT NULL,
  `process_name` varchar(20) DEFAULT NULL,
  `datalength` varchar(20) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `processmonitor` */

LOCK TABLES `processmonitor` WRITE;

insert  into `processmonitor`(`owner_id`,`process_id`,`process_name`,`datalength`) values (0,22,'ss','bbb'),(1,2,'bbb','vv');

UNLOCK TABLES;

/*Table structure for table `record` */

CREATE TABLE `record` (
  `Record_ID` int(11) NOT NULL AUTO_INCREMENT,
  `User_ID` int(11) NOT NULL,
  `Resource_ID` int(11) NOT NULL,
  `Operation_Object` varchar(45) NOT NULL,
  `Operation_ID` varchar(45) NOT NULL,
  `Operation_Time` datetime NOT NULL,
  `Operation_Code` varchar(45) NOT NULL,
  `Operation_Result` varchar(45) NOT NULL,
  `Illegality` bit(1) NOT NULL,
  `Other_Info` varchar(45) DEFAULT NULL,
  `ThreadName` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`Record_ID`)
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=utf8;

/*Data for the table `record` */

LOCK TABLES `record` WRITE;

insert  into `record`(`Record_ID`,`User_ID`,`Resource_ID`,`Operation_Object`,`Operation_ID`,`Operation_Time`,`Operation_Code`,`Operation_Result`,`Illegality`,`Other_Info`,`ThreadName`) values (1,1,1,'1','1','2017-04-15 00:00:00','1','1','\0','1',NULL),(2,1,1,'1','1','2017-04-15 00:00:00','2','2','\0','1',NULL),(4,13,1,'3','3','2017-04-15 00:00:00','2','2','\0','1',NULL),(5,3,1,'c:\\78.txt','12313','2017-04-15 00:00:00','12313','permit','','2017-4-15 00:00:00',NULL),(6,3,1,'C:ÄãºÃ.txt','','2017-04-16 22:06:25','','!!!','\0','!!!',NULL),(7,3,1,'C:\\\\ÄãºÃ.txt','','2017-04-16 22:38:39','','!!!','\0','!!!',NULL),(8,3,1,'C:\\ÄãºÃ.txt','','2017-04-16 22:41:20','','!!!','\0','!!!',NULL),(9,3,1,'C:\\你好.txt','','2017-04-16 22:43:52','','!!!','\0','!!!',NULL),(10,3,1,'C:\\你好.txt','0','2017-04-16 23:07:58','1492355278','!!!','\0','!!!',NULL),(11,3,1,'C:\\你好.txt','2419904','2017-04-17 17:37:48','1492421868','!!!','\0','?',NULL);

UNLOCK TABLES;

/*Table structure for table `resource` */

CREATE TABLE `resource` (
  `Resource_ID` int(11) NOT NULL,
  `Resource_Type` varchar(20) NOT NULL,
  `Resource_TableName` varchar(45) NOT NULL,
  PRIMARY KEY (`Resource_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `resource` */

LOCK TABLES `resource` WRITE;

insert  into `resource`(`Resource_ID`,`Resource_Type`,`Resource_TableName`) values (1,'File','file');

UNLOCK TABLES;

/*Table structure for table `resource_jurisdiction` */

CREATE TABLE `resource_jurisdiction` (
  `Resource_ID` int(11) NOT NULL,
  `Jurisdiction_ID` int(11) NOT NULL,
  `ID` int(11) NOT NULL,
  `Security_Level` int(11) NOT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `resource_jurisdiction` */

LOCK TABLES `resource_jurisdiction` WRITE;

UNLOCK TABLES;

/*Table structure for table `role` */

CREATE TABLE `role` (
  `Role_ID` int(11) NOT NULL,
  `Role_Type` varchar(45) NOT NULL,
  PRIMARY KEY (`Role_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `role` */

LOCK TABLES `role` WRITE;

insert  into `role`(`Role_ID`,`Role_Type`) values (1,'System'),(2,'Advanced'),(3,'pipa');

UNLOCK TABLES;

/*Table structure for table `role_jurisdiction` */

CREATE TABLE `role_jurisdiction` (
  `Role_ID` int(11) NOT NULL,
  `Jurisdiction_ID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `role_jurisdiction` */

LOCK TABLES `role_jurisdiction` WRITE;

insert  into `role_jurisdiction`(`Role_ID`,`Jurisdiction_ID`) values (1,2),(1,3),(1,4),(1,5),(1,6),(1,13),(1,7),(1,9),(1,11),(1,1),(3,3),(3,5),(3,4),(3,1),(3,2),(3,11),(3,9);

UNLOCK TABLES;

/*Table structure for table `security_level` */

CREATE TABLE `security_level` (
  `Security_Level` int(11) NOT NULL,
  `Security_Level_Discription` varchar(45) NOT NULL,
  PRIMARY KEY (`Security_Level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `security_level` */

LOCK TABLES `security_level` WRITE;

insert  into `security_level`(`Security_Level`,`Security_Level_Discription`) values (1,'System_File'),(2,'Advanced_File'),(3,'Private_File'),(4,'Common_File'),(5,'Public_File');

UNLOCK TABLES;

/*Table structure for table `securitylevel_change_logic` */

CREATE TABLE `securitylevel_change_logic` (
  `Securitylevel_change_logiccol_id` int(11) NOT NULL,
  `User_security_level` int(11) DEFAULT NULL,
  `Old_security_level` int(11) NOT NULL,
  `New_security_level` int(11) NOT NULL,
  `Permit` int(11) NOT NULL,
  `IsOwner` int(11) NOT NULL,
  `result_description` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`Securitylevel_change_logiccol_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `securitylevel_change_logic` */

LOCK TABLES `securitylevel_change_logic` WRITE;

insert  into `securitylevel_change_logic`(`Securitylevel_change_logiccol_id`,`User_security_level`,`Old_security_level`,`New_security_level`,`Permit`,`IsOwner`,`result_description`) values (1,0,-1,-1,1,-1,'验证通过'),(6,1,0,0,1,-1,'验证通过'),(10,0,1,0,0,-1,'非管理员不能操作系统文件!'),(11,4,0,0,0,-1,'游客不具备该权限!'),(16,0,0,1,0,-1,'非管理员不能把该文件修改为系统文件!'),(21,0,0,2,0,-1,'非管理员不能把该文件修改为高级文件!'),(26,0,3,0,0,0,'非管理员和文件属主不能执行该操作!'),(31,0,3,0,1,1,'验证通过'),(46,0,0,0,0,-1,'默认该行为被禁止，如果需要可以联系管理员！');

UNLOCK TABLES;

/*Table structure for table `system_config` */

CREATE TABLE `system_config` (
  `load_type` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `system_config` */

LOCK TABLES `system_config` WRITE;

UNLOCK TABLES;

/*Table structure for table `user_level` */

CREATE TABLE `user_level` (
  `iduser_level` int(11) NOT NULL,
  `description` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`iduser_level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `user_level` */

LOCK TABLES `user_level` WRITE;

UNLOCK TABLES;

/*Table structure for table `user_role` */

CREATE TABLE `user_role` (
  `User_ID` int(11) NOT NULL,
  `Role_ID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `user_role` */

LOCK TABLES `user_role` WRITE;

insert  into `user_role`(`User_ID`,`Role_ID`) values (0,1),(2,1),(3,3);

UNLOCK TABLES;

/*Table structure for table `users` */

CREATE TABLE `users` (
  `User_ID` int(10) NOT NULL,
  `Security_Level` int(10) NOT NULL,
  `User_Name` varchar(45) NOT NULL,
  `Create_Time` varchar(20) DEFAULT NULL,
  `Illegelity_Operation_Number` int(11) DEFAULT NULL,
  `Private_Key` varchar(45) DEFAULT '0',
  `Validaty_Period` varchar(20) DEFAULT NULL,
  PRIMARY KEY (`User_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `users` */

LOCK TABLES `users` WRITE;

insert  into `users`(`User_ID`,`Security_Level`,`User_Name`,`Create_Time`,`Illegelity_Operation_Number`,`Private_Key`,`Validaty_Period`) values (0,1,'root','2017-02-23 00:00:00',0,'1','2017-02-22 00:00:00'),(2,1,'pipixia',' ',0,'6',''),(3,2,'pipa',' ',0,'6','');

UNLOCK TABLES;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
