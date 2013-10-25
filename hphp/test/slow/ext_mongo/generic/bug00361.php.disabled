<?php
require_once __DIR__."/../utils/server.inc";
$m = mongo_standalone();

$hosts = $m->getHosts();
if ($hosts && is_array($hosts)) {
    echo "ok\n";
}

$host = current($hosts);
echo $host["host"], ":", $host["port"], "\n";
?>