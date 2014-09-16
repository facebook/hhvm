<?php
// find out TCP port that nobody listens on
for ($port = 1; $port <= 65535; $port++) {
    $fp = @fsockopen('127.0.0.1', $port);
    if ($fp === false) {
        break;
    }
    else {
        fclose($fp);
    }
}
// now test redis session handler on that port
// we expect not able to connect error, but no other warnings or notices
ini_set('session.save_handler', 'redis');
ini_set('session.save_path', "127.0.0.1:$port");
try {
    session_start();
}
catch (Exception $e) {
}
