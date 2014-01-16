<?php
require_once __DIR__."/../utils/server.inc";

$s = new MongoShellServer();
$rs = $s->getReplicaSetConfig(true);
$creds = $s->getCredentials();

$options = array(
    'db' => 'admin',
    'username' => $creds['admin']->username,
    'password' => $creds['admin']->password,
);


try {
    $mc = new MongoClient($rs['dsn'], $options + array('replicaSet' => $rs['rsname']));
    echo "ok\n";
} catch(Exception $e) {
    var_dump(get_class($e), $e->getMessage());
}

?>