<?php
require_once __DIR__."/../utils/server.inc";

MongoLog::setLevel(MongoLog::INFO);
MongoLog::setModule(MongoLog::PARSE);
$host = MongoShellServer::getUnixStandaloneInfo();
try {
    $mc = new MongoClient($host);
    echo "ok\n";
} catch(Exception $e) {
    var_dump(get_class($e), $e->getMessage());
}

?>