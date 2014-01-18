<?php
require_once __DIR__."/../utils/server.inc";

$dsn = MongoShellServer::getStandaloneInfo();

try {
    new MongoClient($dsn, array('readPreferenceTags' => ','));
} catch (MongoConnectionException $e) {
    printf("error message: %s\n", $e->getMessage());
    printf("error code: %d\n", $e->getCode());
}

try {
    new MongoClient('mongodb://' . $dsn . '/?readPreferenceTags=,');
} catch (MongoConnectionException $e) {
    printf("error message: %s\n", $e->getMessage());
    printf("error code: %d\n", $e->getCode());
}

?>