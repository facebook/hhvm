CREATE TABLE `hphp_run` (
  `id` int(11) NOT NULL auto_increment,
  `branch` varchar(255) NOT NULL default '',
  `revision` int(11) NOT NULL default '0',
  `file` int(11) NOT NULL default '0',
  `line` int(11) NOT NULL default '0',
  `byte` int(11) NOT NULL default '0',
  `program` int(11) NOT NULL default '0',
  `function` int(11) NOT NULL default '0',
  `class` int(11) NOT NULL default '0',
  `types` text NOT NULL,
  `time` int(11) NOT NULL default '0',
  `created` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `committed` tinyint(4) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `hphp_dep` (
  `id` int(11) NOT NULL auto_increment,
  `run` int(11) NOT NULL default '0',
  `program` varchar(255) NOT NULL default '',
  `kind` varchar(255) NOT NULL default '',
  `parent` varchar(255) NOT NULL default '',
  `parent_file` varchar(255) NOT NULL default '',
  `parent_line` int(11) NOT NULL default '0',
  `child` varchar(255) NOT NULL default '',
  `child_file` varchar(255) NOT NULL default '',
  `child_line` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `program` (`run`,`program`),
  KEY `parent` (`run`,`kind`,`parent`,`child`),
  KEY `child` (`run`,`kind`,`child`,`parent`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `hphp_err` (
  `id` int(11) NOT NULL auto_increment,
  `run` int(11) NOT NULL default '0',
  `program` varchar(255) NOT NULL default '',
  `kind` varchar(255) NOT NULL default '',
  `construct` bigint(20) NOT NULL default '0',
  `file1` varchar(255) NOT NULL default '',
  `line1` int(11) NOT NULL default '0',
  `file2` varchar(255) NOT NULL default '',
  `line2` int(11) NOT NULL default '0',
  `expected_type` int(11) NOT NULL default '0',
  `actual_type` int(11) NOT NULL default '0',
  `data` varchar(255) NOT NULL default '',
  `suppressed` int(11) NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `program` (`run`,`program`),
  KEY `kind` (`run`,`kind`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

