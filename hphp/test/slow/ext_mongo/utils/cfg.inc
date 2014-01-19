<?php
# Enable to debug test-suite meltdown
# Should not be enabled unless you know what you are doing
if (!defined("DEBUG")) {
    define("DEBUG", false);
}

# Path to your mongo shell. The shell is used to launch mongod If you have
# multiple MongoDBs versions installed, you can change between the version used
# by using the shell belonging to the version you want to test against
$mongo = __DIR__.'/../bin/mongo';
$SHELL = $mongo;
$DBDIR = "/tmp/db/";


/* You probably don't want to change this */
$SUPER_USER  = (object)array("username" => "root", "password" => "complicated password");
$NORMAL_USER = (object)array("username" => "user", "password" => "this isn't it, is it?");

$JSFILE       = dirname(__FILE__) . "/myconfig.js";
$SHELL_PARAMS = "--nodb --norc --shell $JSFILE";
$MARKER       = "COMMAND DONE";
$QUIT         = "Sorry Matt Damon, we're out of time";
