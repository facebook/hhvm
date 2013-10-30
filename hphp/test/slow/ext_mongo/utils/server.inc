<?php

class MongoShellServer {
    protected $fp;
    protected $log = "";
    public function __construct() {
        $fp = fsockopen("localhost", 8000, $errno, $errstr, 2);
        if (!$fp) {
            throw new Exception($errstr, $errno);
        }
        $this->fp = $fp;
    }

    public function makeShard($mongos = 4, $rsOptions = array(), $rsSettings = array()) {
        $this->setDBDIR();
        $rsOptions = json_encode($rsOptions);
        $rsSettings = json_encode($rsSettings);
        return $this->verifyWrite("initShard($mongos, $rsOptions, $rsSettings)");
    }

    public function makeReplicaset($members = 4, $port = 28000, $rsSettings, $auth = false) {
        if ($auth) {
            // Git doesn't store permissions, but mongod needs this to be strict
            chmod($auth, 0700);
        }
        $members = json_encode($members);
        $rsSettings = json_encode($rsSettings);
        $auth = json_encode($auth);
        $creds = $this->getCredentials();
        $root = json_encode($creds["admin"]);
        $user = json_encode($creds["user"]);
        $this->setDBDIR();
        return $this->verifyWrite("initRS($members, $port, $rsSettings, $auth, $root, $user)");
    }

    public function makeStandalone($port = 30000, $auth = false) {
        $creds = $this->getCredentials();
        $auth = json_encode($auth);
        $root = json_encode($creds["admin"]);
        $user = json_encode($creds["user"]);
        $this->setDBDIR();
        return $this->verifyWrite("initStandalone($port, $auth, $root, $user)");
    }

    public function makeBridge($port = 30000, $delay = 1000) {
        return $this->verifyWrite("initBridge($port,$delay)");
    }

    public function killMaster() {
        return $this->verifyWrite("killMaster()");
    }

    public function setMaintenanceForSecondaries($maintenance) {
        $maintenance = json_encode((boolean) $maintenance);
        return $this->verifyWrite("setMaintenanceForSecondaries($maintenance)");
    }

    public function restartMaster() {
        return $this->verifyWrite("restartMaster()");
    }

    public function awaitSecondaryNodes() {
        return $this->verifyWrite("awaitSecondaryNodes()");
    }

    protected function verifyWrite($command) {
        $md5 = md5($command);
        $r = fwrite($this->fp, "print('$md5')\n$command\nprint('$md5')\n");
        $retval = "";
        $verified = false;
        while(!feof($this->fp)) {
            $this->log .= $line = fgets($this->fp);
            if (trim($line) == $md5) {
                if ($verified) {
                    return $retval;
                }
                $verified = true;
                continue;
            }
            if ($verified) {
                $retval .= $line;
            }
        }
        return false;
    }

    public function getReplicaSetConfig($auth = false) {
        $authjs = json_encode($auth);
        $config = json_decode($this->verifyWrite("getReplicaSetConfig($authjs)"), true);
        if (!$config) {
            throw new DebugException(($auth ? "Authenticated " : "") . "Replicaset not initialized", $this->log);
        }
        return self::prettifyRSInfo($config);
    }

    public function getShardConfig() {
        $json = $this->verifyWrite("getShardConfig()");
        $retval = json_decode($json);
        if (!$retval) {
            throw new DebugException("Mongos not initialized", $this->log);
        }
        return $retval;
    }

    public function getUnixStandaloneConfig() {
        $jsauth = json_encode(false);
        $host = $this->verifyWrite("getStandaloneConfig($jsauth)");
        $host = trim($host);
        if ($host == "null") {
            throw new DebugException(($auth ? "Authenticated " : "") . "Standalone server not initialized", $this->log);
        }
        list($hostname, $port) = explode(":", $host);

        return "/tmp/mongodb-$port.sock";
    }
    public function getStandaloneConfig($auth = false) {
        $jsauth = json_encode($auth);
        $host = $this->verifyWrite("getStandaloneConfig($jsauth)");
        $host = trim($host);
        if ($host == "null") {
            throw new DebugException(($auth ? "Authenticated " : "") . "Standalone server not initialized", $this->log);
        }
        return $host;
    }

    public function getServerVersion($which = "STANDALONE") {
        switch($which) {
        case "STANDALONE":
            $buildinfo = $this->verifyWrite("getBuildInfo()");
            break;
        default:
            throw new Exception("Getting the server version for $which isn't implemented yet, fix it!");
        }

        $buildinfo = json_decode($buildinfo, true);
        return $buildinfo["version"];
    }

    public function getBridgeConfig() {
        $host = $this->verifyWrite("getBridgeConfig()");
        $host = trim($host);
        if (!$host || $host == "null") {
            throw new DebugException("mongobridge server not initialized", $this->log);
        }
        return $host;
    }


    public function addStandaloneUser($db, $username, $password) {
        $data   = $this->_prepAddUser($db, $username, $password);
        $retval = $this->verifyWrite("addStandaloneUser({$data["adminjs"]}, {$data["userjs"]})");
        return $retval;
    }

    public function addReplicasetUser($db, $username, $password) {
        $data   = $this->_prepAddUser($db, $username, $password);
        $retval = $this->verifyWrite("addReplicasetUser({$data["adminjs"]}, {$data["userjs"]})");
        return $retval;
    }

    public function _prepAddUser($db, $username, $password) {
        $creds = $this->getCredentials();
        $admin = $creds["admin"];
        $admin->db = "admin";

        $newuser = (object)array(
            "db"       => $db,
            "username" => $username,
            "password" => $password,
        );


        $adminjs = json_encode($admin);
        $userjs  = json_encode($newuser);

        return array(
            "adminjs" => $adminjs,
            "userjs"  => $userjs,
        );
    }

    public function setDBDIR() {
        include dirname(__FILE__) . "/cfg.inc";
        $dbdirjs = json_encode($DBDIR);
        $retval = $this->verifyWrite("setDBDIR($dbdirjs)");
        return $retval;

    }

    public function getDBDIR($json = false) {
        include dirname(__FILE__) . "/cfg.inc";

        return $json ? json_encode($DBDIR) : $DBDIR;
    }

    public function getCredentials() {
        include dirname(__FILE__) . "/cfg.inc";
        return array(
            "admin" => $SUPER_USER,
            "user"  => $NORMAL_USER,
        );
    }

    public function close() {
        if ($this->fp) {
            fwrite($this->fp, "quit\n");
            fclose($this->fp);
        }
        $this->fp = NULL;
    }

    public function __destruct() {
        return $this->close();
    }


    public static function getReplicaSetInfo($auth = false) {
        $o = new self;
        $config = $o->getReplicaSetConfig($auth);
        $o->close();
        return $config;
    }

    public static function getShardInfo() {
        $o = new self;
        $config = $o->getShardConfig();
        $o->close();
        return $config;
    }

    public static function getStandaloneInfo($auth = false) {
        $o = new self;
        $config = $o->getStandaloneConfig($auth);
        $o->close();
        return trim($config);
    }

    public static function getUnixStandaloneInfo() {
        $o = new self;
        $config = $o->getUnixStandaloneConfig();
        $o->close();
        return trim($config);
    }

    public static function getBridgeInfo() {
        $o = new self;
        $config = $o->getBridgeConfig();
        $o->close();
        return trim($config);
    }


    protected static function prettifyRSInfo($config) {
        $rsname = $config["_id"];
        $hosts  = array();
        if (!isset($config["members"])) {
            var_dump($config);
        }
        foreach($config["members"] as $k => $v) {
            $hosts[] = $v["host"];
        }

        return array(
            "dsn"    => join(",", $hosts),
            "rsname" => $rsname,
            "hosts"  => $hosts,
        );
    }
}

class DebugException extends Exception {
    public function __construct($msg, $log) {
        parent::__construct($msg);
        $this->log = $log;
    }
    function getMongoDLog() {
        return $this->log;
    }
}


function getServerVersion($mc = null) {
    $mc->selectDB(dbname())->command(array('buildinfo'=>true));
}



/* BC Layer for old tests */

function mongo() {
    $config = MongoShellServer::getReplicaSetInfo();
    return new Mongo($config["dsn"], array("replicaSet" => $config["rsname"]));
}
function old_mongo() {
    return mongo();
}
function new_mongo($unused1 = null, $unused2 = null, $unused3 = null, $options = array()) {
    $config = MongoShellServer::getReplicaSetInfo();
    return new MongoClient($config["dsn"], array("replicaSet" => $config["rsname"])+$options);
}

function old_mongo_standalone() {
    return mongo_standalone();
}
function mongo_standalone($unused1 = null, $unused2 = null, $unused3 = null, $options = array()) {
    $dsn = MongoShellServer::getStandaloneInfo();
    return new Mongo($dsn, $options);
}
function new_mongo_standalone($unused1 = null, $unused2 = null, $unused3 = null, $options = array()) {
    $dsn = MongoShellServer::getStandaloneInfo();
    return new MongoClient($dsn, $options);
}
function dbname() {
    return "test";
}
function hostname() {
    $config = MongoShellServer::getReplicaSetInfo();
    list($host, $port) = explode(":", $config["hosts"][0]);
    return $host;
}
function standalone_hostname() {
    $dsn = MongoShellServer::getStandaloneInfo();
    list($host, $port) = explode(":", $dsn);
    return $host;
}
function standalone_port() {
    $dsn = MongoShellServer::getStandaloneInfo();
    list($host, $port) = explode(":", $dsn);
    return $port;
}
function port() {
    $config = MongoShellServer::getReplicaSetInfo();
    list($host, $port) = explode(":", $config["hosts"][0]);
    return $port;
}
function rsname() {
    $config = MongoShellServer::getReplicaSetInfo();
    return $config["rsname"];
}
function logCallback($module, $level, $message)
{
    global $_LOG_CALLBACK_REGEX;

    if ($_LOG_CALLBACK_REGEX) {
        if (preg_match($_LOG_CALLBACK_REGEX, $message)) {
            echo $message, "\n";
        }
    } else {
        echo $message, "\n";
    }
}

function printLogs($module = MongoLog::ALL, $level = MongoLog::ALL, $containing = null)
{
    global $_LOG_CALLBACK_REGEX;

    MongoLog::setModule($module);
    MongoLog::setLevel($level);

    if ($containing) {
        $_LOG_CALLBACK_REGEX = $containing;
    }
    MongoLog::setCallback("logCallback");
}

/*
$config = MongoShellServer::getReplicaSetInfo();
list($REPLICASET_SECONDARY, $REPLICASET_SECONDARY_PORT) = explode(":", $config["hosts"][2]);
list($REPLICASET_PRIMARY, $REPLICASET_PRIMARY_PORT) = explode(":", $config["hosts"][0]);
 */

