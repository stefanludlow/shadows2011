-- MySQL dump 10.10
--
-- Host: localhost    Database: shadows
-- ------------------------------------------------------
-- Server version	5.0.22

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Current Database: `shadows`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `shadows` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `shadows`;

--
-- Table structure for table `account_donations`
--

CREATE TABLE `account_donations` (
  `contributor` varchar(255) default NULL,
  `currency` varchar(255) default NULL,
  `amount` float default NULL,
  `deposited` float default NULL,
  `timestamp` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `account_referrals`
--

CREATE TABLE `account_referrals` (
  `account` varchar(255) default NULL,
  `referrer` varchar(255) default NULL,
  `timestamp` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `account_withdrawals`
--

CREATE TABLE `account_withdrawals` (
  `withdrawer` varchar(255) default NULL,
  `amount` float default NULL,
  `reason` varchar(255) default NULL,
  `timestamp` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `announcements`
--

CREATE TABLE `announcements` (
  `author` varchar(255) default NULL,
  `entry` text,
  `datestamp` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `application_wait_times`
--

CREATE TABLE `application_wait_times` (
  `wait_time` int(10) unsigned NOT NULL default '0',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP,
  KEY `timestamp` (`timestamp`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `banned_sites`
--

CREATE TABLE `banned_sites` (
  `site` varchar(255) NOT NULL default '',
  `banned_by` varchar(255) default NULL,
  `banned_on` int(11) default NULL,
  `banned_until` int(11) NOT NULL default '0',
  PRIMARY KEY  (`site`,`banned_until`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `boards`
--

CREATE TABLE `boards` (
  `board_name` varchar(75) default NULL,
  `post_number` int(10) unsigned NOT NULL default '0',
  `subject` varchar(75) default NULL,
  `author` varchar(75) default NULL,
  `ooc_date` varchar(75) default NULL,
  `ic_date` varchar(75) default NULL,
  `message` text,
  `timestamp` int(11) default NULL,
  KEY `board_name` (`board_name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `building_log`
--

CREATE TABLE `building_log` (
  `author` varchar(255) default NULL,
  `entry` text,
  `datestamp` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `building_totals`
--

CREATE TABLE `building_totals` (
  `rooms` int(11) default NULL,
  `objects` int(11) default NULL,
  `mobiles` int(11) default NULL,
  `crafts` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `changed_helpfiles`
--

CREATE TABLE `changed_helpfiles` (
  `staffer` varchar(255) default NULL,
  `change_type` varchar(255) default NULL,
  `category` varchar(255) default NULL,
  `name` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `clan_assignments`
--

CREATE TABLE `clan_assignments` (
  `clan_name` varchar(255) default NULL,
  `imm_name` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `code_changelog`
--

CREATE TABLE `code_changelog` (
  `file_list` text,
  `history` text,
  `timestamp` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `coding_log`
--

CREATE TABLE `coding_log` (
  `author` varchar(255) default NULL,
  `entry` text,
  `datestamp` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `contributions`
--

CREATE TABLE `contributions` (
  `name` varchar(255) default NULL,
  `id` int(11) NOT NULL auto_increment,
  `category` varchar(255) default NULL,
  `contributor` varchar(255) default NULL,
  `filename` varchar(255) default NULL,
  `date` int(11) default NULL,
  `uploaded_by` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `copyover_aiming`
--

CREATE TABLE `copyover_aiming` (
  `aimer_id` int(11) default NULL,
  `target_id` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `copyover_animations`
--

CREATE TABLE `copyover_animations` (
  `animator` int(11) default NULL,
  `target` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `copyover_crafts`
--

CREATE TABLE `copyover_crafts` (
  `crafter_id` int(11) default NULL,
  `craft` varchar(255) default NULL,
  `phase` int(11) default NULL,
  `timer` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `copyover_delays`
--

CREATE TABLE `copyover_delays` (
  `char_id` int(11) default NULL,
  `delay` int(11) default NULL,
  `delay_type` int(11) default NULL,
  `delay_info1` int(11) default NULL,
  `delay_info2` int(11) default NULL,
  `delay_who` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `copyover_doors`
--

CREATE TABLE `copyover_doors` (
  `room` int(11) default NULL,
  `direction` int(11) default NULL,
  `state` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `copyover_fighting`
--

CREATE TABLE `copyover_fighting` (
  `fighter_id` int(11) default NULL,
  `target_id` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `copyover_followers`
--

CREATE TABLE `copyover_followers` (
  `follower_id` int(11) default NULL,
  `leader_id` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `copyover_sighted_targets`
--

CREATE TABLE `copyover_sighted_targets` (
  `sighter_id` int(11) default NULL,
  `target_id` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `copyover_subduers`
--

CREATE TABLE `copyover_subduers` (
  `subduer` int(11) default NULL,
  `target` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `crafts`
--

CREATE TABLE `crafts` (
  `craft` varchar(255) default NULL,
  `subcraft` varchar(255) default NULL,
  `command` varchar(255) default NULL,
  `opening` smallint(6) default NULL,
  `required_skills` varchar(255) default NULL,
  `clanned` smallint(6) default NULL,
  `race` smallint(6) default NULL,
  `opens_for` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `donation_tracking`
--

CREATE TABLE `donation_tracking` (
  `donations` float default NULL,
  `spent` float default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `downloads`
--

CREATE TABLE `downloads` (
  `host` varchar(255) default NULL,
  `account` varchar(255) default NULL,
  `timestamp` int(11) default NULL,
  `filename` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `failed_logins`
--

CREATE TABLE `failed_logins` (
  `account_name` varchar(255) default NULL,
  `hostname` varchar(255) default NULL,
  `timestamp` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_auth_access`
--

CREATE TABLE `forum_auth_access` (
  `group_id` mediumint(8) NOT NULL default '0',
  `forum_id` smallint(5) unsigned NOT NULL default '0',
  `auth_view` tinyint(1) NOT NULL default '0',
  `auth_read` tinyint(1) NOT NULL default '0',
  `auth_post` tinyint(1) NOT NULL default '0',
  `auth_reply` tinyint(1) NOT NULL default '0',
  `auth_edit` tinyint(1) NOT NULL default '0',
  `auth_delete` tinyint(1) NOT NULL default '0',
  `auth_sticky` tinyint(1) NOT NULL default '0',
  `auth_announce` tinyint(1) NOT NULL default '0',
  `auth_vote` tinyint(1) NOT NULL default '0',
  `auth_pollcreate` tinyint(1) NOT NULL default '0',
  `auth_attachments` tinyint(1) NOT NULL default '0',
  `auth_mod` tinyint(1) NOT NULL default '0',
  KEY `group_id` (`group_id`),
  KEY `forum_id` (`forum_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_banlist`
--

CREATE TABLE `forum_banlist` (
  `ban_id` mediumint(8) unsigned NOT NULL auto_increment,
  `ban_userid` mediumint(8) NOT NULL default '0',
  `ban_ip` varchar(8) NOT NULL default '',
  `ban_email` varchar(255) default NULL,
  PRIMARY KEY  (`ban_id`),
  KEY `ban_ip_user_id` (`ban_ip`,`ban_userid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_categories`
--

CREATE TABLE `forum_categories` (
  `cat_id` mediumint(8) unsigned NOT NULL auto_increment,
  `cat_title` varchar(100) default NULL,
  `cat_order` mediumint(8) unsigned NOT NULL default '0',
  PRIMARY KEY  (`cat_id`),
  KEY `cat_order` (`cat_order`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_config`
--

CREATE TABLE `forum_config` (
  `config_name` varchar(255) NOT NULL default '',
  `config_value` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`config_name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_confirm`
--

CREATE TABLE `forum_confirm` (
  `confirm_id` char(32) NOT NULL default '',
  `session_id` char(32) NOT NULL default '',
  `code` char(6) NOT NULL default '',
  PRIMARY KEY  (`session_id`,`confirm_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_disallow`
--

CREATE TABLE `forum_disallow` (
  `disallow_id` mediumint(8) unsigned NOT NULL auto_increment,
  `disallow_username` varchar(25) NOT NULL default '',
  PRIMARY KEY  (`disallow_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_forum_lock`
--

CREATE TABLE `forum_forum_lock` (
  `lock_id` mediumint(8) unsigned NOT NULL auto_increment,
  `forum_id` smallint(5) unsigned NOT NULL default '0',
  `lock_days` smallint(5) unsigned NOT NULL default '0',
  `lock_freq` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`lock_id`),
  KEY `forum_id` (`forum_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_forum_prune`
--

CREATE TABLE `forum_forum_prune` (
  `prune_id` mediumint(8) unsigned NOT NULL auto_increment,
  `forum_id` smallint(5) unsigned NOT NULL default '0',
  `prune_days` smallint(5) unsigned NOT NULL default '0',
  `prune_freq` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`prune_id`),
  KEY `forum_id` (`forum_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_forums`
--

CREATE TABLE `forum_forums` (
  `forum_id` smallint(5) unsigned NOT NULL default '0',
  `cat_id` mediumint(8) unsigned NOT NULL default '0',
  `forum_name` varchar(150) default NULL,
  `forum_desc` text,
  `forum_status` tinyint(4) NOT NULL default '0',
  `forum_order` mediumint(8) unsigned NOT NULL default '1',
  `forum_posts` mediumint(8) unsigned NOT NULL default '0',
  `forum_topics` mediumint(8) unsigned NOT NULL default '0',
  `forum_last_post_id` mediumint(8) unsigned NOT NULL default '0',
  `prune_next` int(11) default NULL,
  `prune_enable` tinyint(1) NOT NULL default '0',
  `auth_view` tinyint(2) NOT NULL default '0',
  `auth_read` tinyint(2) NOT NULL default '0',
  `auth_post` tinyint(2) NOT NULL default '0',
  `auth_reply` tinyint(2) NOT NULL default '0',
  `auth_edit` tinyint(2) NOT NULL default '0',
  `auth_delete` tinyint(2) NOT NULL default '0',
  `auth_sticky` tinyint(2) NOT NULL default '0',
  `auth_announce` tinyint(2) NOT NULL default '0',
  `auth_vote` tinyint(2) NOT NULL default '0',
  `auth_pollcreate` tinyint(2) NOT NULL default '0',
  `auth_attachments` tinyint(2) NOT NULL default '0',
  `forum_clan` varchar(255) default NULL,
  `lock_enable` tinyint(1) NOT NULL default '0',
  `lock_next` int(11) default NULL,
  PRIMARY KEY  (`forum_id`),
  KEY `forums_order` (`forum_order`),
  KEY `cat_id` (`cat_id`),
  KEY `forum_last_post_id` (`forum_last_post_id`),
  KEY `forum_id` (`forum_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_groups`
--

CREATE TABLE `forum_groups` (
  `group_id` mediumint(8) NOT NULL auto_increment,
  `group_type` tinyint(4) NOT NULL default '1',
  `group_name` varchar(40) NOT NULL default '',
  `group_description` varchar(255) NOT NULL default '',
  `group_moderator` mediumint(8) NOT NULL default '0',
  `group_single_user` tinyint(1) NOT NULL default '1',
  PRIMARY KEY  (`group_id`),
  KEY `group_single_user` (`group_single_user`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_posts`
--

CREATE TABLE `forum_posts` (
  `post_id` mediumint(8) unsigned NOT NULL auto_increment,
  `topic_id` mediumint(8) unsigned NOT NULL default '0',
  `forum_id` smallint(5) unsigned NOT NULL default '0',
  `poster_id` mediumint(8) NOT NULL default '0',
  `post_time` int(11) NOT NULL default '0',
  `poster_ip` varchar(8) NOT NULL default '',
  `post_username` varchar(25) default NULL,
  `enable_bbcode` tinyint(1) NOT NULL default '1',
  `enable_html` tinyint(1) NOT NULL default '0',
  `enable_smilies` tinyint(1) NOT NULL default '1',
  `enable_sig` tinyint(1) NOT NULL default '1',
  `post_edit_time` int(11) default NULL,
  `post_edit_count` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`post_id`),
  KEY `forum_id` (`forum_id`),
  KEY `topic_id` (`topic_id`),
  KEY `poster_id` (`poster_id`),
  KEY `post_time` (`post_time`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_posts_text`
--

CREATE TABLE `forum_posts_text` (
  `post_id` mediumint(8) unsigned NOT NULL default '0',
  `bbcode_uid` varchar(10) NOT NULL default '',
  `post_subject` varchar(60) default NULL,
  `post_text` text,
  PRIMARY KEY  (`post_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_privmsgs`
--

CREATE TABLE `forum_privmsgs` (
  `privmsgs_id` mediumint(8) unsigned NOT NULL auto_increment,
  `privmsgs_type` tinyint(4) NOT NULL default '0',
  `privmsgs_subject` varchar(255) NOT NULL default '0',
  `privmsgs_from_userid` mediumint(8) NOT NULL default '0',
  `privmsgs_to_userid` mediumint(8) NOT NULL default '0',
  `privmsgs_date` int(11) NOT NULL default '0',
  `privmsgs_ip` varchar(8) NOT NULL default '',
  `privmsgs_enable_bbcode` tinyint(1) NOT NULL default '1',
  `privmsgs_enable_html` tinyint(1) NOT NULL default '0',
  `privmsgs_enable_smilies` tinyint(1) NOT NULL default '1',
  `privmsgs_attach_sig` tinyint(1) NOT NULL default '1',
  PRIMARY KEY  (`privmsgs_id`),
  KEY `privmsgs_from_userid` (`privmsgs_from_userid`),
  KEY `privmsgs_to_userid` (`privmsgs_to_userid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_privmsgs_text`
--

CREATE TABLE `forum_privmsgs_text` (
  `privmsgs_text_id` mediumint(8) unsigned NOT NULL default '0',
  `privmsgs_bbcode_uid` varchar(10) NOT NULL default '0',
  `privmsgs_text` text,
  PRIMARY KEY  (`privmsgs_text_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_ranks`
--

CREATE TABLE `forum_ranks` (
  `rank_id` smallint(5) unsigned NOT NULL auto_increment,
  `rank_title` varchar(50) NOT NULL default '',
  `rank_min` mediumint(8) NOT NULL default '0',
  `rank_special` tinyint(1) default '0',
  `rank_image` varchar(255) default NULL,
  PRIMARY KEY  (`rank_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_search_results`
--

CREATE TABLE `forum_search_results` (
  `search_id` int(11) unsigned NOT NULL default '0',
  `session_id` varchar(32) NOT NULL default '',
  `search_array` text NOT NULL,
  PRIMARY KEY  (`search_id`),
  KEY `session_id` (`session_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_search_wordlist`
--

CREATE TABLE `forum_search_wordlist` (
  `word_text` varchar(50) character set latin1 collate latin1_bin NOT NULL default '',
  `word_id` mediumint(8) unsigned NOT NULL auto_increment,
  `word_common` tinyint(1) unsigned NOT NULL default '0',
  PRIMARY KEY  (`word_text`),
  KEY `word_id` (`word_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_search_wordmatch`
--

CREATE TABLE `forum_search_wordmatch` (
  `post_id` mediumint(8) unsigned NOT NULL default '0',
  `word_id` mediumint(8) unsigned NOT NULL default '0',
  `title_match` tinyint(1) NOT NULL default '0',
  KEY `post_id` (`post_id`),
  KEY `word_id` (`word_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_sessions`
--

CREATE TABLE `forum_sessions` (
  `session_id` char(32) NOT NULL default '',
  `session_user_id` mediumint(8) NOT NULL default '0',
  `session_start` int(11) NOT NULL default '0',
  `session_time` int(11) NOT NULL default '0',
  `session_ip` char(8) NOT NULL default '0',
  `session_page` int(11) NOT NULL default '0',
  `session_logged_in` tinyint(1) NOT NULL default '0',
  `session_admin` tinyint(2) NOT NULL default '0',
  PRIMARY KEY  (`session_id`),
  KEY `session_user_id` (`session_user_id`),
  KEY `session_id_ip_user_id` (`session_id`,`session_ip`,`session_user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_smilies`
--

CREATE TABLE `forum_smilies` (
  `smilies_id` smallint(5) unsigned NOT NULL auto_increment,
  `code` varchar(50) default NULL,
  `smile_url` varchar(100) default NULL,
  `emoticon` varchar(75) default NULL,
  PRIMARY KEY  (`smilies_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_themes`
--

CREATE TABLE `forum_themes` (
  `themes_id` mediumint(8) unsigned NOT NULL auto_increment,
  `template_name` varchar(30) NOT NULL default '',
  `style_name` varchar(30) NOT NULL default '',
  `head_stylesheet` varchar(100) default NULL,
  `body_background` varchar(100) default NULL,
  `body_bgcolor` varchar(6) default NULL,
  `body_text` varchar(6) default NULL,
  `body_link` varchar(6) default NULL,
  `body_vlink` varchar(6) default NULL,
  `body_alink` varchar(6) default NULL,
  `body_hlink` varchar(6) default NULL,
  `tr_color1` varchar(6) default NULL,
  `tr_color2` varchar(6) default NULL,
  `tr_color3` varchar(6) default NULL,
  `tr_class1` varchar(25) default NULL,
  `tr_class2` varchar(25) default NULL,
  `tr_class3` varchar(25) default NULL,
  `th_color1` varchar(6) default NULL,
  `th_color2` varchar(6) default NULL,
  `th_color3` varchar(6) default NULL,
  `th_class1` varchar(25) default NULL,
  `th_class2` varchar(25) default NULL,
  `th_class3` varchar(25) default NULL,
  `td_color1` varchar(6) default NULL,
  `td_color2` varchar(6) default NULL,
  `td_color3` varchar(6) default NULL,
  `td_class1` varchar(25) default NULL,
  `td_class2` varchar(25) default NULL,
  `td_class3` varchar(25) default NULL,
  `fontface1` varchar(50) default NULL,
  `fontface2` varchar(50) default NULL,
  `fontface3` varchar(50) default NULL,
  `fontsize1` tinyint(4) default NULL,
  `fontsize2` tinyint(4) default NULL,
  `fontsize3` tinyint(4) default NULL,
  `fontcolor1` varchar(6) default NULL,
  `fontcolor2` varchar(6) default NULL,
  `fontcolor3` varchar(6) default NULL,
  `span_class1` varchar(25) default NULL,
  `span_class2` varchar(25) default NULL,
  `span_class3` varchar(25) default NULL,
  `img_size_poll` smallint(5) unsigned default NULL,
  `img_size_privmsg` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`themes_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_themes_name`
--

CREATE TABLE `forum_themes_name` (
  `themes_id` smallint(5) unsigned NOT NULL default '0',
  `tr_color1_name` char(50) default NULL,
  `tr_color2_name` char(50) default NULL,
  `tr_color3_name` char(50) default NULL,
  `tr_class1_name` char(50) default NULL,
  `tr_class2_name` char(50) default NULL,
  `tr_class3_name` char(50) default NULL,
  `th_color1_name` char(50) default NULL,
  `th_color2_name` char(50) default NULL,
  `th_color3_name` char(50) default NULL,
  `th_class1_name` char(50) default NULL,
  `th_class2_name` char(50) default NULL,
  `th_class3_name` char(50) default NULL,
  `td_color1_name` char(50) default NULL,
  `td_color2_name` char(50) default NULL,
  `td_color3_name` char(50) default NULL,
  `td_class1_name` char(50) default NULL,
  `td_class2_name` char(50) default NULL,
  `td_class3_name` char(50) default NULL,
  `fontface1_name` char(50) default NULL,
  `fontface2_name` char(50) default NULL,
  `fontface3_name` char(50) default NULL,
  `fontsize1_name` char(50) default NULL,
  `fontsize2_name` char(50) default NULL,
  `fontsize3_name` char(50) default NULL,
  `fontcolor1_name` char(50) default NULL,
  `fontcolor2_name` char(50) default NULL,
  `fontcolor3_name` char(50) default NULL,
  `span_class1_name` char(50) default NULL,
  `span_class2_name` char(50) default NULL,
  `span_class3_name` char(50) default NULL,
  PRIMARY KEY  (`themes_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_topics`
--

CREATE TABLE `forum_topics` (
  `topic_id` mediumint(8) unsigned NOT NULL auto_increment,
  `forum_id` smallint(8) unsigned NOT NULL default '0',
  `topic_title` char(60) NOT NULL default '',
  `topic_poster` mediumint(8) NOT NULL default '0',
  `topic_time` int(11) NOT NULL default '0',
  `topic_views` mediumint(8) unsigned NOT NULL default '0',
  `topic_replies` mediumint(8) unsigned NOT NULL default '0',
  `topic_status` tinyint(3) NOT NULL default '0',
  `topic_vote` tinyint(1) NOT NULL default '0',
  `topic_type` tinyint(3) NOT NULL default '0',
  `topic_first_post_id` mediumint(8) unsigned NOT NULL default '0',
  `topic_last_post_id` mediumint(8) unsigned NOT NULL default '0',
  `topic_moved_id` mediumint(8) unsigned NOT NULL default '0',
  PRIMARY KEY  (`topic_id`),
  KEY `forum_id` (`forum_id`),
  KEY `topic_moved_id` (`topic_moved_id`),
  KEY `topic_status` (`topic_status`),
  KEY `topic_type` (`topic_type`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_topics_watch`
--

CREATE TABLE `forum_topics_watch` (
  `topic_id` mediumint(8) unsigned NOT NULL default '0',
  `user_id` mediumint(8) NOT NULL default '0',
  `notify_status` tinyint(1) NOT NULL default '0',
  KEY `topic_id` (`topic_id`),
  KEY `user_id` (`user_id`),
  KEY `notify_status` (`notify_status`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_user_group`
--

CREATE TABLE `forum_user_group` (
  `group_id` mediumint(8) NOT NULL default '0',
  `user_id` mediumint(8) NOT NULL default '0',
  `user_pending` tinyint(1) default NULL,
  KEY `group_id` (`group_id`),
  KEY `user_id` (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_users`
--

CREATE TABLE `forum_users` (
  `user_id` mediumint(8) NOT NULL default '0',
  `user_active` tinyint(1) default '1',
  `username` varchar(25) NOT NULL default '',
  `roleplay_points` int(10) unsigned default '0',
  `user_password` varchar(32) NOT NULL default '',
  `user_last_ip` varchar(80) default NULL,
  `user_subscription` smallint(5) unsigned default '1',
  `user_color` smallint(5) unsigned default '0',
  `user_session_time` int(11) NOT NULL default '0',
  `user_session_page` smallint(5) NOT NULL default '0',
  `user_lastvisit` int(11) NOT NULL default '0',
  `user_regdate` int(11) NOT NULL default '0',
  `user_level` tinyint(4) default '0',
  `user_posts` mediumint(8) unsigned NOT NULL default '0',
  `user_timezone` decimal(5,2) NOT NULL default '0.00',
  `user_style` tinyint(4) default NULL,
  `user_lang` varchar(255) default NULL,
  `user_dateformat` varchar(14) NOT NULL default 'd M Y H:i',
  `user_new_privmsg` smallint(5) unsigned NOT NULL default '0',
  `user_unread_privmsg` smallint(5) unsigned NOT NULL default '0',
  `user_last_privmsg` int(11) NOT NULL default '0',
  `user_emailtime` int(11) default NULL,
  `user_viewemail` tinyint(1) default '0',
  `user_attachsig` tinyint(1) default NULL,
  `user_allowhtml` tinyint(1) default '1',
  `user_allowbbcode` tinyint(1) default '1',
  `user_allowsmile` tinyint(1) default '1',
  `user_allowavatar` tinyint(1) NOT NULL default '1',
  `user_allow_pm` tinyint(1) NOT NULL default '1',
  `user_allow_viewonline` tinyint(1) NOT NULL default '1',
  `user_notify` tinyint(1) NOT NULL default '0',
  `user_notify_pm` tinyint(1) NOT NULL default '1',
  `user_popup_pm` tinyint(1) NOT NULL default '1',
  `user_rank` int(11) default '0',
  `user_avatar` varchar(100) default NULL,
  `user_avatar_type` tinyint(4) NOT NULL default '0',
  `user_email` varchar(255) default NULL,
  `user_icq` varchar(15) default NULL,
  `user_website` varchar(100) default NULL,
  `user_from` varchar(100) default NULL,
  `user_sig` text,
  `user_sig_bbcode_uid` varchar(10) default NULL,
  `user_aim` varchar(255) default NULL,
  `user_yim` varchar(255) default NULL,
  `user_msnm` varchar(255) default NULL,
  `user_occ` varchar(100) default NULL,
  `user_interests` varchar(255) default NULL,
  `user_actkey` varchar(32) default NULL,
  `user_newpasswd` varchar(32) default NULL,
  `account_flags` int(11) NOT NULL default '0',
  `tms_votes` int(10) unsigned default '0',
  `last_tms_vote` int(10) unsigned default '0',
  `mm_votes` int(10) unsigned NOT NULL default '0',
  `last_mm_vote` int(10) unsigned NOT NULL default '0',
  `tmc_votes` int(10) unsigned NOT NULL default '0',
  `last_tmc_vote` int(10) unsigned NOT NULL default '0',
  `downloaded_code` int(10) unsigned default NULL,
  `last_rpp` int(10) unsigned NOT NULL default '0',
  `pass` varchar(32) default NULL,
  PRIMARY KEY  (`user_id`),
  KEY `user_session_time` (`user_session_time`),
  KEY `username` (`username`),
  KEY `user_last_ip` (`user_last_ip`),
  KEY `user_email` (`user_email`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_vote_desc`
--

CREATE TABLE `forum_vote_desc` (
  `vote_id` mediumint(8) unsigned NOT NULL auto_increment,
  `topic_id` mediumint(8) unsigned NOT NULL default '0',
  `vote_text` text NOT NULL,
  `vote_start` int(11) NOT NULL default '0',
  `vote_length` int(11) NOT NULL default '0',
  PRIMARY KEY  (`vote_id`),
  KEY `topic_id` (`topic_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_vote_results`
--

CREATE TABLE `forum_vote_results` (
  `vote_id` mediumint(8) unsigned NOT NULL default '0',
  `vote_option_id` tinyint(4) unsigned NOT NULL default '0',
  `vote_option_text` varchar(255) NOT NULL default '',
  `vote_result` int(11) NOT NULL default '0',
  KEY `vote_option_id` (`vote_option_id`),
  KEY `vote_id` (`vote_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_vote_voters`
--

CREATE TABLE `forum_vote_voters` (
  `vote_id` mediumint(8) unsigned NOT NULL default '0',
  `vote_user_id` mediumint(8) NOT NULL default '0',
  `vote_user_ip` char(8) NOT NULL default '',
  KEY `vote_id` (`vote_id`),
  KEY `vote_user_id` (`vote_user_id`),
  KEY `vote_user_ip` (`vote_user_ip`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `forum_words`
--

CREATE TABLE `forum_words` (
  `word_id` mediumint(8) unsigned NOT NULL auto_increment,
  `word` char(100) NOT NULL default '',
  `replacement` char(100) NOT NULL default '',
  PRIMARY KEY  (`word_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `helpfiles`
--

CREATE TABLE `helpfiles` (
  `name` varchar(250) NOT NULL default '',
  `category` varchar(250) NOT NULL default '',
  `entry` text,
  `related_entries` varchar(250) default NULL,
  `required_level` int(10) unsigned default NULL,
  `last_changed` varchar(255) default NULL,
  `changed_by` varchar(255) default NULL,
  PRIMARY KEY  (`name`,`category`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `hobbitmail`
--

CREATE TABLE `hobbitmail` (
  `account` varchar(255) default NULL,
  `flags` int(10) unsigned default '0',
  `from_line` varchar(255) default NULL,
  `from_account` varchar(255) default NULL,
  `sent_date` varchar(255) default NULL,
  `subject` varchar(255) default NULL,
  `message` text,
  `timestamp` int(10) unsigned NOT NULL default '0',
  `id` int(11) NOT NULL auto_increment,
  `to_line` varchar(255) default NULL,
  PRIMARY KEY  (`id`),
  KEY `account` (`account`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `massmail_messages`
--

CREATE TABLE `massmail_messages` (
  `subject` varchar(255) default NULL,
  `type` varchar(10) default NULL,
  `message` text,
  `username` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `mob_resets`
--

CREATE TABLE `mob_resets` (
  `zone` int(11) default NULL,
  `cmd_no` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `new_crafts`
--

CREATE TABLE `new_crafts` (
  `command` varchar(255) default NULL,
  `subcraft` varchar(255) default NULL,
  `craft` varchar(255) default NULL,
  `creator` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `newsletter_issue`
--

CREATE TABLE `newsletter_issue` (
  `volume` int(11) default NULL,
  `issue` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `newsletter_stats`
--

CREATE TABLE `newsletter_stats` (
  `accepted_apps` int(11) NOT NULL default '0',
  `declined_apps` int(11) NOT NULL default '0',
  `resolved_typos` int(11) NOT NULL default '0',
  `resolved_bugs` int(11) NOT NULL default '0',
  `pc_deaths` int(11) NOT NULL default '0',
  `new_accounts` int(11) NOT NULL default '0',
  `guest_logins` int(11) NOT NULL default '0',
  `most_online` int(11) NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `newsletters`
--

CREATE TABLE `newsletters` (
  `timestamp` int(11) default NULL,
  `newsletter` text
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `player_journals`
--

CREATE TABLE `player_journals` (
  `name` varchar(75) default NULL,
  `post_number` int(10) unsigned NOT NULL default '0',
  `subject` varchar(75) default NULL,
  `author` varchar(75) default NULL,
  `date` varchar(75) default NULL,
  `message` text,
  KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `player_notes`
--

CREATE TABLE `player_notes` (
  `name` varchar(75) default NULL,
  `post_number` int(10) unsigned NOT NULL default '0',
  `subject` varchar(75) default NULL,
  `author` varchar(75) default NULL,
  `date` varchar(75) default NULL,
  `message` text,
  `flags` int(11) default NULL,
  `timestamp` int(11) default NULL,
  KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `player_writing`
--

CREATE TABLE `player_writing` (
  `db_key` int(11) default NULL,
  `author` varchar(255) default NULL,
  `page` int(11) default NULL,
  `date` varchar(255) default NULL,
  `ink` varchar(255) default NULL,
  `language` int(11) default NULL,
  `script` int(11) default NULL,
  `skill` int(11) default NULL,
  `writing` text,
  `timestamp` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `players_online`
--

CREATE TABLE `players_online` (
  `account` varchar(255) default NULL,
  `name` varchar(255) default NULL,
  `hostname` varchar(255) default NULL,
  `room` int(11) default NULL,
  `port` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `plot_log`
--

CREATE TABLE `plot_log` (
  `author` varchar(255) default NULL,
  `entry` text,
  `datestamp` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `professions`
--

CREATE TABLE `professions` (
  `name` varchar(255) default NULL,
  `skill_list` varchar(255) default NULL,
  `id` int(10) unsigned NOT NULL auto_increment,
  `picked` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `queued_reviews`
--

CREATE TABLE `queued_reviews` (
  `char_name` varchar(255) default NULL,
  `reviewer` varchar(255) default NULL,
  `new_keywords` varchar(255) default NULL,
  `new_sdesc` varchar(255) default NULL,
  `new_ldesc` varchar(255) default NULL,
  `new_desc` text,
  `new_comment` text,
  `accepted` smallint(5) unsigned default '0',
  `response` text
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `queued_startups`
--

CREATE TABLE `queued_startups` (
  `port` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `queued_submissions`
--

CREATE TABLE `queued_submissions` (
  `account` varchar(30) default NULL,
  `type` varchar(30) default NULL,
  `field1` text,
  `field2` text,
  `field3` text,
  `field4` text,
  `field5` text,
  `field6` text,
  `field7` text,
  `field8` text,
  `id` int(10) unsigned NOT NULL auto_increment,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `race_rpp_data`
--

CREATE TABLE `race_rpp_data` (
  `race_name` varchar(255) default NULL,
  `point_req` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `races`
--

CREATE TABLE `races` (
  `name` varchar(255) default NULL,
  `id` int(11) NOT NULL auto_increment,
  `rpp_cost` smallint(6) default '0',
  `description` text,
  `starting_loc` int(11) default '0',
  `pc_race` tinyint(1) default '0',
  `affects` int(11) default '0',
  `body_proto` smallint(6) default '0',
  `size` smallint(6) default '0',
  `str_mod` smallint(6) default '0',
  `con_mod` smallint(6) default '0',
  `dex_mod` smallint(6) default '0',
  `agi_mod` smallint(6) default '0',
  `int_mod` smallint(6) default '0',
  `wil_mod` smallint(6) default '0',
  `aur_mod` smallint(6) default '0',
  `min_age` smallint(6) default '12',
  `max_age` smallint(6) default '75',
  `min_height` smallint(6) default '48',
  `max_height` smallint(6) default '78',
  `fem_ht_diff` int(11) default '0',
  `native_tongue` smallint(6) default '0',
  `skill_mods` text,
  `created_by` varchar(255) default NULL,
  `last_modified` int(11) default NULL,
  `max_hit` smallint(6) default '0',
  `max_move` smallint(6) default '0',
  `armor` smallint(6) default '0',
  `wear_head` int(11) default NULL,
  `wear_neck_1` int(11) default NULL,
  `wear_about` int(11) default NULL,
  `wear_body` int(11) default NULL,
  `wear_back` int(11) default NULL,
  `wear_shoulder_r` int(11) default NULL,
  `wear_shoulder_l` int(11) default NULL,
  `wear_arms` int(11) default NULL,
  `wear_hands` int(11) default NULL,
  `wear_waist` int(11) default NULL,
  `wear_belt_1` int(11) default NULL,
  `wear_belt_2` int(11) default NULL,
  `wear_legs` int(11) default NULL,
  `wear_feet` int(11) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `reboot_mobiles`
--

CREATE TABLE `reboot_mobiles` (
  `vnum` int(11) default NULL,
  `room` int(11) default NULL,
  `coldload_id` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `registered_characters`
--

CREATE TABLE `registered_characters` (
  `account_name` varchar(255) default NULL,
  `character_name` varchar(255) default NULL,
  `regdate` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `resolved_hosts`
--

CREATE TABLE `resolved_hosts` (
  `ip` varchar(255) NOT NULL default '',
  `hostname` varchar(255) default NULL,
  `timestamp` int(11) default NULL,
  PRIMARY KEY  (`ip`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `reviews_in_progress`
--

CREATE TABLE `reviews_in_progress` (
  `char_name` varchar(255) default NULL,
  `reviewer` varchar(255) default NULL,
  `timestamp` int(10) unsigned default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `saveroom_objs`
--

CREATE TABLE `saveroom_objs` (
  `room` int(11) default NULL,
  `wc` int(11) default NULL,
  `pos` int(11) default NULL,
  `objstack` int(11) default NULL,
  `obj_vnum` int(11) default NULL,
  `name` varchar(255) default NULL,
  `short_desc` varchar(255) default NULL,
  `long_desc` varchar(255) default NULL,
  `full_desc` text,
  `desc_keys` varchar(255) default NULL,
  `book_title` varchar(255) default NULL,
  `title_skill` int(11) default NULL,
  `title_language` int(11) default NULL,
  `title_script` int(11) default NULL,
  `loaded` int(11) default NULL,
  `extraflags` int(11) default NULL,
  `oval0` int(11) default NULL,
  `oval1` int(11) default NULL,
  `oval2` int(11) default NULL,
  `oval3` int(11) default NULL,
  `oval4` int(11) default NULL,
  `oval5` int(11) default NULL,
  `weight` int(11) default NULL,
  `size` int(11) default NULL,
  `count` int(11) default NULL,
  `timer` int(11) default NULL,
  `clock` int(11) default NULL,
  `morphto` int(11) default NULL,
  `morphTime` int(11) default NULL,
  `varcolor` varchar(255) default NULL,
  `omotestr` varchar(255) default NULL,
  `wounds` text,
  `lodged` text,
  `affects` text
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `server_statistics`
--

CREATE TABLE `server_statistics` (
  `last_crash` int(11) default NULL,
  `last_reboot` int(11) default NULL,
  `max_players` int(11) default NULL,
  `morgul_arena` int(10) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `skills`
--

CREATE TABLE `skills` (
  `name` varchar(255) default NULL,
  `id` int(10) unsigned NOT NULL auto_increment,
  `skill_type` int(10) unsigned NOT NULL default '0',
  `rpp_restriction` int(10) unsigned NOT NULL default '0',
  `restricted_races` varchar(255) default NULL,
  `ov` varchar(255) default NULL,
  `lv` varchar(255) default NULL,
  `cap` varchar(255) default NULL,
  `func_info` varchar(255) default NULL,
  `created_by` varchar(255) default NULL,
  `last_modified` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `special_orders`
--

CREATE TABLE `special_orders` (
  `id` int(11) NOT NULL auto_increment,
  `char_name` varchar(255) default NULL,
  `shopkeep_vnum` int(11) default NULL,
  `item_vnum` int(11) default NULL,
  `var_color` varchar(255) default NULL,
  `quantity` int(11) default NULL,
  `total_cost` int(11) default NULL,
  `order_placed` int(11) default NULL,
  `order_ready` int(11) default NULL,
  `redeemed` int(11) default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `special_roles`
--

CREATE TABLE `special_roles` (
  `summary` varchar(255) default NULL,
  `poster` varchar(255) default NULL,
  `date` varchar(255) default NULL,
  `cost` int(10) unsigned default NULL,
  `body` text,
  `timestamp` int(10) unsigned NOT NULL default '0',
  `role_id` int(11) NOT NULL auto_increment,
  PRIMARY KEY  (`role_id`),
  KEY `role_id` (`role_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `spells`
--

CREATE TABLE `spells` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) default NULL,
  `base_cost` int(11) default NULL,
  `sphere` smallint(6) default NULL,
  `circle` smallint(6) default NULL,
  `ch_echo` varchar(255) default NULL,
  `self_echo` varchar(255) default NULL,
  `vict_echo` varchar(255) default NULL,
  `room_echo` varchar(255) default NULL,
  `spell_description` text,
  `target_type` int(11) default NULL,
  `save_type` int(11) default NULL,
  `save_for` int(11) default NULL,
  `damage_form` int(11) default NULL,
  `affect1` int(11) default NULL,
  `dice1` varchar(20) default NULL,
  `act1` varchar(255) default NULL,
  `fade1` varchar(255) default NULL,
  `affect2` int(11) default NULL,
  `dice2` varchar(20) default NULL,
  `act2` varchar(255) default NULL,
  `fade2` varchar(255) default NULL,
  `affect3` int(11) default NULL,
  `dice3` varchar(20) default NULL,
  `act3` varchar(255) default NULL,
  `fade3` varchar(255) default NULL,
  `affect4` int(11) default NULL,
  `dice4` varchar(20) default NULL,
  `act4` varchar(255) default NULL,
  `fade4` varchar(255) default NULL,
  `affect5` int(11) default NULL,
  `dice5` varchar(20) default NULL,
  `act5` varchar(255) default NULL,
  `fade5` varchar(255) default NULL,
  `author` varchar(255) default NULL,
  `last_modified` int(10) unsigned default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `staff_roster`
--

CREATE TABLE `staff_roster` (
  `name` varchar(255) default NULL,
  `title` varchar(255) default NULL,
  `seniority` int(11) default NULL,
  `dept` varchar(255) NOT NULL default '',
  `weight` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `stayput_mobiles`
--

CREATE TABLE `stayput_mobiles` (
  `vnum` int(11) default NULL,
  `room` int(11) default NULL,
  `coldload_id` int(11) default NULL,
  `morph_time` int(13) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `testimonials`
--

CREATE TABLE `testimonials` (
  `quote` text,
  `name` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `tracks`
--

CREATE TABLE `tracks` (
  `room` int(10) unsigned default NULL,
  `race` int(10) unsigned default NULL,
  `from_dir` int(10) unsigned default NULL,
  `to_dir` int(10) unsigned default NULL,
  `hours_passed` int(10) unsigned default NULL,
  `speed` int(10) unsigned default NULL,
  `flags` int(10) unsigned default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `unneeded_helpfiles`
--

CREATE TABLE `unneeded_helpfiles` (
  `name` varchar(255) default NULL,
  `datestamp` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `virtual_boards`
--

CREATE TABLE `virtual_boards` (
  `board_name` varchar(75) NOT NULL default '',
  `post_number` int(10) unsigned NOT NULL default '0',
  `subject` varchar(75) default NULL,
  `author` varchar(75) default NULL,
  `date_posted` varchar(75) default NULL,
  `message` text,
  `timestamp` int(11) default NULL,
  KEY `board_name_idx` (`board_name`),
  KEY `post_number` (`post_number`),
  KEY `timestamp` (`timestamp`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `vote_notifications`
--

CREATE TABLE `vote_notifications` (
  `ip_addr` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `vote_tracking`
--

CREATE TABLE `vote_tracking` (
  `ip_address` varchar(255) default NULL,
  `site_id` varchar(10) default NULL,
  `datestamp` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `website_errors`
--

CREATE TABLE `website_errors` (
  `errno` int(10) unsigned NOT NULL default '0',
  `request_uri` varchar(255) default NULL,
  `referrer` varchar(255) default NULL,
  `user_agent` varchar(255) default NULL,
  `requesting_host` varchar(255) default NULL,
  `timestamp` int(10) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `website_links`
--

CREATE TABLE `website_links` (
  `url` varchar(255) default NULL,
  `name` varchar(255) default NULL,
  `category` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `website_log`
--

CREATE TABLE `website_log` (
  `author` varchar(255) default NULL,
  `entry` text,
  `datestamp` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `website_referrals`
--

CREATE TABLE `website_referrals` (
  `site` varchar(255) default NULL,
  `page` varchar(255) default NULL,
  `host` varchar(255) default NULL,
  `timestamp` int(11) default NULL,
  KEY `site` (`site`),
  KEY `host` (`host`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `website_sessions`
--

CREATE TABLE `website_sessions` (
  `username` varchar(255) default NULL,
  `random_string` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Current Database: `shadows_pfiles`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `shadows_pfiles` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `shadows_pfiles`;

--
-- Table structure for table `clans`
--

CREATE TABLE `clans` (
  `id` smallint(5) unsigned NOT NULL auto_increment,
  `name` varchar(32) default NULL,
  `long_name` varchar(80) default NULL,
  `zone` smallint(6) NOT NULL default '0',
  `member_obj` mediumint(8) unsigned NOT NULL default '0',
  `leader_obj` mediumint(8) unsigned NOT NULL default '0',
  `omni_obj` mediumint(8) unsigned NOT NULL default '0',
  UNIQUE KEY `id` (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `dreams`
--

CREATE TABLE `dreams` (
  `name` varchar(255) NOT NULL default '',
  `dreamed` int(11) default NULL,
  `dream` text
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `morgul_reserves_20060401`
--

CREATE TABLE `morgul_reserves_20060401` (
  `name` varchar(255) NOT NULL default '',
  `account` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `pfiles`
--

CREATE TABLE `pfiles` (
  `name` varchar(255) NOT NULL default '',
  `keywords` varchar(255) default NULL,
  `account` varchar(255) default NULL,
  `sdesc` varchar(255) default NULL,
  `ldesc` varchar(255) default NULL,
  `description` text,
  `msg` text,
  `create_comment` text,
  `create_state` int(11) default NULL,
  `nanny_state` int(11) default NULL,
  `role` int(11) default NULL,
  `role_summary` varchar(255) default NULL,
  `role_body` text,
  `role_date` varchar(255) default NULL,
  `role_poster` varchar(75) default NULL,
  `role_cost` int(11) default NULL,
  `app_cost` int(11) default NULL,
  `level` int(11) default NULL,
  `sex` int(11) default NULL,
  `deity` int(11) default NULL,
  `race` int(11) default NULL,
  `room` int(11) default NULL,
  `str` int(11) default NULL,
  `intel` int(11) default NULL,
  `wil` int(11) default NULL,
  `con` int(11) default NULL,
  `dex` int(11) default NULL,
  `aur` int(11) default NULL,
  `agi` int(11) default NULL,
  `start_str` int(11) default NULL,
  `start_intel` int(11) default NULL,
  `start_wil` int(11) default NULL,
  `start_con` int(11) default NULL,
  `start_dex` int(11) default NULL,
  `start_aur` int(11) default NULL,
  `start_agi` int(11) default NULL,
  `played` int(11) default NULL,
  `birth` int(11) default NULL,
  `time` int(11) default NULL,
  `offense` int(11) default NULL,
  `hit` int(11) default NULL,
  `maxhit` int(11) default NULL,
  `nat_attack_type` int(11) default NULL,
  `move` int(11) default NULL,
  `maxmove` int(11) default NULL,
  `circle` int(11) default NULL,
  `ppoints` int(11) default NULL,
  `fightmode` int(11) default NULL,
  `color` int(11) default NULL,
  `speaks` int(11) default NULL,
  `flags` int(11) default NULL,
  `plrflags` int(11) default NULL,
  `boatvnum` int(11) default NULL,
  `speed` int(11) default NULL,
  `mountspeed` int(11) default NULL,
  `sleepneeded` int(11) default NULL,
  `autotoll` int(11) default NULL,
  `coldload` int(10) unsigned NOT NULL default '0',
  `affectedby` int(11) default NULL,
  `affects` text,
  `age` int(11) default NULL,
  `intoxication` int(11) default NULL,
  `hunger` int(11) default NULL,
  `thirst` int(11) default NULL,
  `height` int(11) default NULL,
  `frame` int(11) default NULL,
  `damage` int(11) default NULL,
  `lastregen` int(11) default NULL,
  `lastroom` int(11) default NULL,
  `harness` int(11) default NULL,
  `maxharness` int(11) default NULL,
  `lastlogon` int(11) default NULL,
  `lastlogoff` int(11) default NULL,
  `lastdis` int(11) default NULL,
  `lastconnect` int(11) default NULL,
  `lastdied` int(11) default NULL,
  `hooded` int(11) default NULL,
  `immenter` varchar(255) default NULL,
  `immleave` varchar(255) default NULL,
  `sitelie` varchar(255) default NULL,
  `voicestr` varchar(255) default NULL,
  `clans` text,
  `skills` text,
  `wounds` text,
  `lodged` text,
  `writes` int(11) NOT NULL default '0',
  `profession` int(10) unsigned NOT NULL default '0',
  `was_in_room` int(11) NOT NULL default '0',
  `travelstr` varchar(255) default NULL,
  `last_rpp` int(10) unsigned NOT NULL default '0',
  `bmi` smallint(6) default '0',
  `guardian_mode` int(10) unsigned NOT NULL default '0',
  `hire_storeroom` int(10) unsigned default '0',
  `hire_storeobj` int(10) unsigned default '0',
  `plan` varchar(80) default NULL,
  `goal` varchar(255) default NULL,
  `role_id` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`),
  KEY `account` (`account`),
  KEY `create_state` (`create_state`),
  KEY `account_2` (`account`,`create_state`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Current Database: `shadows_worldfile`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `shadows_worldfile` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `shadows_worldfile`;

--
-- Table structure for table `cues`
--

CREATE TABLE `cues` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `mid` int(10) unsigned default NULL,
  `cue` enum('notes','flags','memory','on_hour','on_time','on_health','on_moves','on_command','on_receive','on_hear','on_nod','on_theft','on_witness','on_engage','on_flee','on_scan','on_hit') default NULL,
  `reflex` varchar(255) default NULL,
  UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Current Database: `server_logs`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `server_logs` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `server_logs`;

--
-- Table structure for table `ip`
--

CREATE TABLE `ip` (
  `account` varchar(32) NOT NULL default '',
  `host` varchar(255) default NULL,
  `ip` varchar(32) NOT NULL default '',
  `firsttime` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `lasttime` timestamp NOT NULL default '0000-00-00 00:00:00',
  `count` int(11) default '0',
  `is_new` tinyint(1) default '0',
  `has_pwd` tinyint(1) default '0',
  `port` int(11) NOT NULL default '0',
  `logins` int(11) NOT NULL default '0',
  `fails` int(11) NOT NULL default '0',
  PRIMARY KEY  (`account`,`ip`,`port`),
  KEY `host` (`host`),
  KEY `ip` (`ip`),
  KEY `account` (`account`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `loggers`
--

CREATE TABLE `loggers` (
  `timestamp` timestamp NOT NULL default '0000-00-00 00:00:00',
  `account` varchar(32) default NULL,
  `squrl` text,
  `ip` varchar(32) default NULL,
  `devsite` tinyint(1) default NULL,
  KEY `acct_ts` (`account`(4),`timestamp`),
  KEY `account` (`account`(4),`timestamp`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `mud`
--

CREATE TABLE `mud` (
  `name` varchar(255) default NULL,
  `account` varchar(255) default NULL,
  `switched_into` varchar(255) default NULL,
  `timestamp` int(11) default '0',
  `port` int(11) default '4500',
  `room` int(11) default '-1',
  `guest` tinyint(1) default '0',
  `immortal` tinyint(1) default '0',
  `error` tinyint(1) default '0',
  `command` varchar(255) default NULL,
  `entry` text,
  `sha_hash` varchar(45) default NULL,
  KEY `ts_idx` (`timestamp`),
  KEY `room_idx` (`room`),
  KEY `name` (`name`),
  KEY `command` (`command`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `receipts`
--

CREATE TABLE `receipts` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `shopkeep` mediumint(8) unsigned NOT NULL default '0',
  `transaction` enum('unknown','sold','bought') NOT NULL default 'unknown',
  `who` varchar(32) default NULL,
  `customer` varchar(255) default NULL,
  `vnum` mediumint(8) unsigned NOT NULL default '0',
  `item` varchar(80) default NULL,
  `qty` tinyint(3) unsigned NOT NULL default '0',
  `cost` mediumint(8) unsigned NOT NULL default '0',
  `room` mediumint(8) unsigned NOT NULL default '0',
  `gametime` datetime default NULL,
  `port` smallint(5) unsigned NOT NULL default '4500',
  PRIMARY KEY  (`id`),
  KEY `shopkeep` (`shopkeep`,`port`),
  KEY `time` (`time`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `shade`
--

CREATE TABLE `shade` (
  `id` int(10) unsigned default NULL,
  `ip` int(10) unsigned default NULL,
  `name` varchar(255) NOT NULL default '',
  UNIQUE KEY `name` (`name`),
  UNIQUE KEY `id` (`id`),
  UNIQUE KEY `ip` (`ip`),
  KEY `id_2` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Current Database: `shadows_ah`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `shadows_ah` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `shadows_ah`;

--
-- Table structure for table `ah_auctions`
--

CREATE TABLE `ah_auctions` (
  `auction_id` int(11) NOT NULL auto_increment,
  `house_id` int(11) NOT NULL,
  `placed_at` int(11) NOT NULL COMMENT 'Timestamp for when the auction was placed.',
  `auction_period` int(11) NOT NULL COMMENT 'Period for which the auction lasts. (In RL hours?)',
  `expires_at` int(11) NOT NULL COMMENT 'Timestamp for when the auction expires.',
  `deposit_paid` int(11) NOT NULL COMMENT 'The deposit put down by the auctioner for the auction.',
  `obj_real_value` int(11) NOT NULL COMMENT 'The "real" value set by the game of the object, in coppers.',
  `min_bid` int(11) NOT NULL,
  `next_bid` int(11) NOT NULL,
  `high_bidder` varchar(255) NOT NULL default 'none',
  `buyout` int(11) NOT NULL,
  `placed_by` varchar(255) NOT NULL default '',
  `placed_by_sdesc` varchar(255) NOT NULL default '',
  `placed_by_account` varchar(255) NOT NULL default '',
  `obj_type` int(11) NOT NULL,
  `obj_short_desc` varchar(255) NOT NULL default '',
  `obj_long_desc` varchar(255) NOT NULL default '',
  `obj_full_desc` text NOT NULL,
  `obj_vnum` int(11) NOT NULL,
  `buyer_pickup` int(11) NOT NULL,
  `seller_pickup` int(11) NOT NULL,
  `bought_out` int(11) NOT NULL,
  `sold_for` int(11) NOT NULL default '0',
  `port` int(11) NOT NULL default '4502',
  `cancelled` int(11) NOT NULL default '0',
  PRIMARY KEY  (`auction_id`),
  KEY `auction_id` (`auction_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `ah_bids`
--

CREATE TABLE `ah_bids` (
  `bid_id` int(11) NOT NULL auto_increment,
  `auction_id` int(11) NOT NULL,
  `bid_amount` int(11) NOT NULL,
  `buyout` bit(1) NOT NULL default '\0',
  `placed_at` int(11) NOT NULL,
  `placed_by` varchar(255) NOT NULL default '',
  `placed_by_sdesc` varchar(255) NOT NULL default '',
  `placed_by_account` varchar(255) NOT NULL default '',
  `port` int(11) NOT NULL default '4502',
  PRIMARY KEY  (`bid_id`),
  KEY `auction_id` (`auction_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `ah_returned_bids`
--

CREATE TABLE `ah_returned_bids` (
  `auction_id` int(11) NOT NULL,
  `amount` int(11) NOT NULL,
  `placed_by` varchar(255) NOT NULL default '',
  `house_id` int(11) NOT NULL,
  `picked_up` smallint(6) NOT NULL default '0',
  `port` int(11) NOT NULL default '4502',
  KEY `auction_id` (`auction_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

