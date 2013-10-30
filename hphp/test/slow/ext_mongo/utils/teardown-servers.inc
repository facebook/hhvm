<?php
include __DIR__."/../utils/cfg.inc";
$fp = fsockopen("localhost", 8000, $errno, $errstr, 2);
if (!$fp) {
    throw new Exception($errstr, $errno);
}
fwrite($fp, $QUIT . "\n");
fflush($fp);
echo "Waiting for everything to spin down again..\n";
echo stream_get_contents($fp);
fflush($fp);
fclose($fp);

