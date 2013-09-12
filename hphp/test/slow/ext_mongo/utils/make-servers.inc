<?php
/*
 * The only bootup specific type of servers set
 *      MONGO_SERVER_[SERVER_TYPE]=yes
 * in your environment before running this script.
 *
 * To bootup all exception a specific type of server set
 *      SKIP_MONGO_SERVER_[SERVER_TYPE]=yes
 */
if (!file_exists(__DIR__."/../utils/cfg.inc")) {
    echo "Please copy ".__DIR__."/../utils/cfg.inc.template to ".__DIR__."/../utils/cfg.inc\n";
    exit(112);
}
require_once __DIR__."/../utils/server.inc";
include __DIR__."/../utils/cfg.inc";

function t() {
    static $last;
    if ($last) {
        $current = microtime(true);
        $retval = $current - $last;
        $last = $current;
        return $retval;
    }
    $last = microtime(true);
}
function makeServer($SERVERS, $server, $bit) {
    echo "Making " . $SERVERS[$bit] . ".. ";
    t();
    switch($bit) {
    case STANDALONE:
        $server->makeStandalone(30000);
        $dsn = $server->getStandaloneConfig();
        break;
    case STANDALONE_AUTH:
        $server->makeStandalone(30100, true);
        $dsn = $server->getStandaloneConfig(true);
        break;
    case STANDALONE_BRIDGE:
        $sc = $server->getStandaloneConfig();
        list($shost, $sport) = explode(":", trim($sc));
        try {
            $server->makeBridge($sport, 1000);
            $dsn = $server->getBridgeConfig();
        } catch(DebugException $e) {
            printf("(%s) %s - Still continuing though, you probably don't have 'mongobridge' installed\n", get_class($e), $e->getMessage());
            //printf("%s\n", $e->getMongoDLog());
            return 0;
        }
        break;
    case MONGOS:
        $rsmembers = array(
            /* Shard 0 */ array(
                array('tags' => array('server' => '0', 'dc' => 'ny')),
                array('tags' => array('server' => '1', 'dc' => 'ny')),
                array('tags' => array('server' => '2', 'dc' => 'sf'), "priority" => 0),
            ),
            /* Shard 1 */ array(
                array('tags' => array('server' => '0', 'dc' => 'ny')),
                array('tags' => array('server' => '1', 'dc' => 'ny', 'theOtherShard' => 'doesntHaveThisTag')),
                array('tags' => array('server' => '2', 'dc' => 'sf'), "priority" => 0),
            ),
        );
        $rssettings = array(
            array(
                "getLastErrorModes" => array(
                    "AnyDC" => array("dc" => 1),
                    "AllDC" => array("dc" => 2),
                    "ALL"   => array("server" => 3),
                ),
            ),
            array(
                "getLastErrorModes" => array(
                    "AnyDC" => array("dc" => 1),
                    "AllDC" => array("dc" => 2),
                    "ALL"   => array("server" => 3),
                    "Broken" => array("theOtherShard" => 1),
                ),
            )
        );
        $retval = $server->makeShard(2, $rsmembers, $rssettings);
        $cfg = $server->getShardConfig();
        $dsn = join(",", $cfg);
        break;
    case REPLICASET:
    case REPLICASET_AUTH:
        $members = array(
            array('tags' => array('server' => '0', 'dc' => 'ny'), "priority" => 42),
            array('tags' => array('server' => '1', 'dc' => 'ny')),
            array('tags' => array('server' => '2', 'dc' => 'sf')),
            array('tags' => array('server' => '3', 'dc' => 'sf'), "priority" => 0),
        );
        $settings = array(
            "getLastErrorModes" => array(
                "AnyDC" => array("dc" => 1),
                "AllDC" => array("dc" => 2),
                "ALL"   => array("server" => 4),
            ),
        );

        if ($bit == REPLICASET) {
            $server->makeReplicaset($members, 30200, $settings);
            $cfg = $server->getReplicaSetConfig();
        } else { /* REPLICASET_AUTH */
            /* Remove the passive server from auth replicasets.
             * We need to have one replicaset without a passive server, see
             * PHP-829 */
            unset($members[3]);
            $settings["getLastErrorModes"]["ALL"]["server"]--;
            $retval = $server->makeReplicaset($members, 30300, $settings, dirname(__FILE__) . "/keyFile");
            $cfg = $server->getReplicaSetConfig(true);
        }
        $dsn = $cfg["dsn"];
        break;
    default:
        var_dump("No idea what to do about $bit");
        exit(32);
    }
    printf("DONE (%.2f secs): %s\n", t(), $dsn);
}

$SERVERS = array(
    "STANDALONE"        => 0x01,
    "STANDALONE_AUTH"   => 0x02,
    "STANDALONE_BRIDGE" => 0x04,
    "MONGOS"            => 0x08,
    "REPLICASET"        => 0x10,
    "REPLICASET_AUTH"   => 0x20,
);

$ALL_SERVERS = 0;
$BOOTSTRAP = 0;
foreach($SERVERS as $server => $bit) {
    define($server, $bit);
    $ALL_SERVERS |= $bit;
}

foreach($SERVERS as $server => $bit) {
    if (getenv("MONGO_SERVER_$server")) {
        $BOOTSTRAP |= $bit;
    }
}
if (!$BOOTSTRAP) {
    $BOOTSTRAP = $ALL_SERVERS;
    foreach($SERVERS as $server => $bit) {
        if (getenv("SKIP_MONGO_SERVER_$server")) {
            $BOOTSTRAP &= ~$bit;
        }
    }
}
function makeDaemon() {
    $pid = pcntl_fork();
    if ($pid > 0) {
        sleep(1);
        echo "Daemon running..\n";
        return;
    }
    if (!$pid) {
        posix_setsid();
        require_once dirname(__FILE__) . "/daemon.inc";
        exit(0);
    }
}

if (!is_dir($DBDIR)) {
    if (!mkdir($DBDIR, 0700, true)) {
        echo "Error creating database directory: $DBDIR\n";
        exit(2);
    }
}

try {
    $server = new MongoShellServer;
} catch(Exception $e) {
    echo "Does't look like the daemon is up and running.. Starting it now\n";
    makeDaemon();
    try {
        $server = new MongoShellServer;
    } catch(Exception $e)  {
        echo $e->getMessage();
        exit(2);
    }
}

foreach($SERVERS as $k => $bit) {
    if ($BOOTSTRAP & $bit) {
        try {
            makeServer(array_flip($SERVERS), $server, $bit);
        } catch(DebugException $e) {
            echo $e->getMessage(), "\n";
            $filename = tempnam(sys_get_temp_dir(), "MONGO-PHP-TESTS");
            file_put_contents($filename, $e->getMongoDLog());
            echo "Debug log from mongod writter to $filename\n";
            $server->close();
            include __DIR__."/../utils/teardown-servers.inc";
            exit(113);
        }
    }
}

$server->close();

echo "We have liftoff \n";

