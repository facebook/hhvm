<?php
require_once __DIR__."/../utils/server.inc";
$c = new_mongo_standalone();
$db = $c->selectDb(dbname());

$func = 
    "function(greeting, name) { ".
        "return greeting+', '+name+', says '+greeter;".
    "}";
$scope = array("greeter" => "Fred");

$code = new MongoCode($func, $scope);

$opts = array('nolock' => false);
$response = $db->execute($code, array("Goodbye", "Joe"), $opts);
var_dump($opts, $response);
?>