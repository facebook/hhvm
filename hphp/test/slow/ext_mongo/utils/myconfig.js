var replTest;
var replTestAuth;
var standaloneTest;
var standaloneTestAuth;
var shardTest;
var shardTestAuth;
var bridgeTest;

/**
 * Initialize a replica set.
 *
 * If the number of servers is even, an additional member will be added to the
 * set as an arbiter process.
 *
 * If the server parameter is an array, its length will determine the number of
 * nodes in the replica set. Additionally, each element is expected to be an
 * object with configuration options for the node (e.g. tags).
 *
 * @param int|array servers Number of servers, or array of server config options
 * @param int       port    Starting port for replica set nodes
 * @param string    keyFile Path to private key file (activates auth if present)
 * @param object    root    Object with username/password fields for "admin" DB user
 * @param object    user    Object with username/password fields for "test" DB user
 * @return ReplSetTest
 */
function initRS(servers, port, rsSettings, keyFile, root, user) {
    servers = typeof servers !== 'undefined' ? servers : 3;
    port = typeof port !== 'undefined' ? port : 28000;

    // Do not reitinialize a replica set
    if ((keyFile && replTestAuth) || (!keyFile && replTest)) {
        return (keyFile ? replTestAuth : replTest);
    }

    // Save serverOpts before extracting servers from the array's length
    if (Array.isArray(servers)) {
        var serverOpts = servers;
        servers = servers.length;
    }

    var addArbiter = (servers % 2 == 0);

    var testOpts = {
        "name": keyFile ? "REPLICASET-AUTH" : "REPLICASET",
        "useHostname": false,
        "useHostName": false,
        "nodes": servers + (addArbiter ? 1 : 0),
        "startPort": port
    };

    var retval = new ReplSetTest(testOpts);
    print("Now setting logpath");
    for ( var i = 0; i < retval.numNodes; i++ ) {
        retval.nodeOptions[ "n" + i ] = Object.merge(retval.nodeOptions[ "n" + i ], {
            "nojournal" : "",
            "nopreallocj": "",
            "quiet": "",
            "oplogSize": 10,
            "logpath": "/tmp/NODE." + (keyFile ? "-AUTH" : "") + i
        });
        if (keyFile) {
            retval.nodeOptions[ "n" + i ].keyFile = keyFile;
        }

        printjson(retval.nodeOptions["n" + i]);
    }
    print("Finish setting logpath");
    retval.startSet();
    var cfg = retval.getReplSetConfig();

    if (addArbiter) {
        cfg.members[servers].arbiterOnly = true;
    }

    // Apply server configuration options, if available
    if (typeof serverOpts !== 'undefined') {
        for (var i = 0; i < servers; i++) {
            cfg.members[i] = Object.extend(cfg.members[i], serverOpts[i]);
        }
        cfg.settings = rsSettings;
    }

    retval.initiate(cfg);
    retval.awaitReplication();

    if (keyFile) {
        retval.getMaster().getDB("admin").addUser(root.username, root.password);
        retval.getMaster().getDB("admin").auth(root.username, root.password);
        retval.getMaster().getDB("test").addUser(user.username, user.password);
        replTestAuth = retval;
    } else {
        replTest = retval;
    }

    retval.getMaster().getDB("test").fixtures.insert({example: "document"});

    return retval;
}

/**
 * Initialize a standalone server.
 *
 * If the number of servers is even, an additional member will be added to the
 * set as an arbiter process.
 *
 * @param int     port Server port
 * @param boolean auth Whether to enable authentication
 * @param object  root Object with username/password fields for "admin" DB user
 * @param object  user Object with username/password fields for "test" DB user
 * @return Mongo
 */
function initStandalone(port, auth, root, user) {
    port = typeof port !== 'undefined' ? port : 27000;

    // Do not reitinialize a standalone server
    if ((auth && standaloneTestAuth) || (!auth && standaloneTest)) {
        return (auth ? standaloneTestAuth : standaloneTest);
    }

    var opts = {
        "dbpath" : MongoRunner.dataPath + port
    };

    if (auth) {
        opts.auth = "";
    }

    var retval = startMongodTest(port, false, false, opts);
    retval.port = port;

    assert.soon(function() {
        try {
            conn = new Mongo("127.0.0.1:" + port);
            return true;
        } catch(e) {
            printjson(e);
        }
        return false;
    }, "unable to connect to mongo program on port " + port, 600 * 1000);

    if (auth) {
        retval.getDB("admin").addUser(root.username, root.password);
        retval.getDB("admin").auth(root.username, root.password);
        retval.getDB("test").addUser(user.username, user.password);
        standaloneTestAuth = retval;
    } else {
        standaloneTest = retval;
    }

    retval.getDB("test").fixtures.insert({example: "document"});
    return retval;
}

/**
 * Initialize a sharded cluster.
 *
 * @return ShardingTest
 */
function initShard(mongoscount, rsOptions, rsSettings) {
    mongoscount = typeof mongoscount !== 'undefined' ? mongoscount : 3;
    rsOptions = typeof rsOptions !== 'undefined' ? rsOptions : [];

    // Do not reitinialize a sharded cluster
    if (shardTest) {
        return shardTest;
    }

    rs = {
        "nodes": 3,
        "logpath": "/tmp/NODE.RS",
        "useHostname": false,
        "useHostName": false,
        "oplogSize": 10
    }

    shardTest = new ShardingTest({
        "name": "SHARDING",
        "useHostname": false,
        "useHostName": false,
        "shards": 2,
        "rs": rs,
        "numReplicas": 2,
        "nopreallocj": true,
        "mongos": mongoscount,
        "other": {
            "mongosOptions": {
                "logpath": "/dev/null"
            }
        }
    });

    if (typeof rsOptions !== 'undefined') {
        cfg = shardTest.rs0.getReplSetConfig();
        for (var i = 0; i < rsOptions[0].length; i++) {
            cfg.members[i] = Object.extend(cfg.members[i], rsOptions[0][i]);
        }
        cfg.settings = rsSettings[0];
        // Version isn't set yet so we can't just ++ it
        cfg.version = 3;
        try {
            shardTest.rs0.getMaster().getDB("admin")._adminCommand({ replSetReconfig : cfg });
            /* Will close all the open connections, we don't care */
        } catch(ex) {}

        cfg = shardTest.rs1.getReplSetConfig();
        for (var i = 0; i < rsOptions[1].length; i++) {
            cfg.members[i] = Object.extend(cfg.members[i], rsOptions[1][i]);
        }
        cfg.settings = rsSettings[1];
        cfg.version = 3;
        try {
            shardTest.rs1.getMaster().getDB("admin")._adminCommand({ replSetReconfig : cfg });
        } catch(ex) {}
    }

    ReplSetTest.awaitRSClientHosts(shardTest.s, shardTest.rs0.getSecondaries(), { ok : true, secondary : true });
    ReplSetTest.awaitRSClientHosts(shardTest.s, shardTest.rs1.getSecondaries(), { ok : true, secondary : true });
    ReplSetTest.awaitRSClientHosts(shardTest.s, shardTest.rs1.getPrimary(), { ok : true, ismaster : true });
    shardTest.rs0.getMaster().getDB("test").fixtures.insert({example: "document"});
    shardTest.rs1.getMaster().getDB("test").fixtures.insert({example: "document"});

    return shardTest;
}

/**
 * Initialize a mongobridge.
 *
 * @return Mongo
 */
function initBridge(port, delay) {
    delay = typeof delay !== 'undefined' ? delay : 300;

    // Do not reinitialize a mongobridge
    if (bridgeTest) {
        return bridgeTest;
    }

    var bridgePort = allocatePorts(1)[0];
    bridgeTest = startMongoProgram("mongobridge", "--port", bridgePort, "--dest", "localhost:" + port, "--delay", delay);
    bridgeTest.port = bridgePort;

    return bridgeTest;
}

/**
 * Get standalone host.
 *
 * @param boolean auth
 * @return string Host/port of standalone server
 */
function getStandaloneConfig(auth) {
    if (auth) {
        return standaloneTestAuth ? standaloneTestAuth.host : null;
    }
    return standaloneTest ? standaloneTest.host : null;
}

/**
 * Get replica set configuration.
 *
 * @param boolean auth
 * @return object
 */
function getReplicaSetConfig(auth) {
    if (auth) {
        return replTestAuth ? replTestAuth.getReplSetConfig() : null;
    }
    return replTest ? replTest.getReplSetConfig() : null;
}

/**
 * Get mongobridge host.
 *
 * @return string Host/port of mongobridge
 */
function getBridgeConfig() {
    return bridgeTest ? bridgeTest.host : null;
}

/**
 * Get sharded cluster hosts.
 *
 * @return array Array of host/port strings for each mongos
 */
function getShardConfig() {
    return shardTest ? [shardTest.s0.host, shardTest.s1.host] : null;
}

/**
 * Stop replica set primary.
 */
function killMaster() {
    // Step down for 5seconds, and prevent elections for 6 seconds so we can do some tests without primary
    printjson(replTest.getMaster().getDB("admin")._adminCommand({ replSetStepDown: 5, force: true }));
    printjson(replTest.getMaster().getDB("admin")._adminCommand({ replSetFreeze: 6 }));
}

/**
 * Set maintenance mode for all secondaries
 */
function setMaintenanceForSecondaries(maintenance) {
    replTest.getMaster(); // Wait for a primary to be selected
    var slaves = replTest.liveNodes.slaves;
    slaves.forEach(function(slave) {
        var isMaster = slave.getDB("admin").runCommand({ismaster: 1});
        var arbiter = isMaster['arbiterOnly'] == undefined ? false : isMaster['arbiterOnly'];
        if (!arbiter) {
            slave.getDB("admin").adminCommand({ replSetMaintenance: maintenance });
        }
    });

    /* If we set secondaries to maintenance mode, wait for them to become
     * unhealthy. Otherwise, wait for healthy secondaries.
     */
    if (maintenance) {
        awaitUnhealthySecondaryNodes();
    } else {
        awaitSecondaryNodes();
    }
}

/**
 * Stop replica set.
 */
function killRS() {
    replTest.stopSet();
}

/**
 * Restart replica set primary.
 */
function restartMaster() {
    // Just wait until we get a master
    printjson(replTest.getMaster());
}

/**
 * Wait until all secondaries are healthy
 */
function awaitSecondaryNodes() {
    printjson(replTest.awaitSecondaryNodes());
}

/**
 * Wait until all secondaries are not healthy
 */
function awaitUnhealthySecondaryNodes() {
    var timeout = 60000;

    replTest.getMaster(); // Wait for a primary to be selected.

    assert.soon(function() {
        replTest.getMaster(); // Reload who the current slaves are.
        var slaves = replTest.liveNodes.slaves;
        var ready = true;
        slaves.forEach(function(slave) {
            var isMaster = slave.getDB("admin").runCommand({ismaster: 1});
            var arbiter = isMaster['arbiterOnly'] == undefined ? false : isMaster['arbiterOnly'];
            if (!arbiter) {
                ready = ready && !isMaster['secondary'];
            }
        });
        return ready;
    }, "Awaiting unhealthy secondaries", 60000);
};

/**
 * Set MongoRunner dataDir and dataPath properties.
 *
 * @param string path
 */
function setDBDIR(path) {
    MongoRunner.dataDir = path;
    MongoRunner.dataPath = path;
}

/**
 * Add a user to standaloneTestAuth.
 *
 * @see _addUser()
 */
function addStandaloneUser(loginUser, newUser) {
    return _addUser(standaloneTestAuth, loginUser, newUser);
}

/**
 * Add a user to replTestAuth.
 *
 * @see _addUser()
 */
function addReplicasetUser(loginUser, newUser) {
    return _addUser(replTestAuth.getMaster(), loginUser, newUser);
}

/**
 * Add a user to the database.
 *
 * The loginUser will be used to authenticate before adding newUser.
 *
 * @param Mongo  conn      Database connection
 * @param object loginUser Object with db/username/password fields
 * @param object newUser   Object with db/username/password fields
 * @return object
 */
function _addUser(conn, loginUser, newUser) {
    conn.getDB(loginUser.db).auth(loginUser.username, loginUser.password);
    return conn.getDB(newUser.db).addUser(newUser.username, newUser.password);
}

/**
 * Shutdown all test servers, replica sets, shard clusters, and bridges.
 *
 * @param function callback Callback to execute after shutdown completes
 */
function shutdownEverything(callback) {
    if (typeof standaloneTest !== "undefined") {
        MongoRunner.stopMongod(standaloneTest.port);
    }
    if (typeof standaloneTestAuth !== "undefined") {
        MongoRunner.stopMongod(standaloneTestAuth.port);
    }
    if (typeof bridgeTest !== "undefined") {
        MongoRunner.stopMongod(bridgeTest.port);
    }
    if (typeof replTest !== "undefined") {
        replTest.stopSet();
    }
    if (typeof replTestAuth !== "undefined") {
        replTestAuth.stopSet();
    }
    if (typeof shardTest !== "undefined") {
        shardTest.stop();
    }
    if (typeof callback === "function") {
        callback();
    }
}

function getBuildInfo() {
    return standaloneTest.adminCommand({buildinfo: 1})
}
