<?php
class none {
  function stream_open()    { echo "open\n";  return true; }
  function stream_write($d) { echo "write\n"; return strlen($d); }
  function stream_close()   { echo "close\n"; }
}
class null extends none {
  function stream_flush()   { echo "flush did something\n"; }
}
class no extends none {
  function stream_flush()   { echo "flush failed\n"; return false; }
}
class yes extends none {
  function stream_flush()   { echo "flush succeeded\n"; return true; }
}
stream_register_wrapper("none", "none");
stream_register_wrapper("null", "null");
stream_register_wrapper("yes", "yes");
stream_register_wrapper("no", "no");

var_dump(copy(__FILE__, "none://this")); echo "\n";
var_dump(copy(__FILE__, "null://this")); echo "\n";
var_dump(copy(__FILE__, "yes://this")); echo "\n";
var_dump(copy(__FILE__, "no://this")); echo "\n";

var_dump(file_put_contents("none://this", "missing")); echo "\n";
var_dump(file_put_contents("null://this", "must return bool")); echo "\n";
var_dump(file_put_contents("yes://this", "explicit success")); echo "\n";
var_dump(file_put_contents("no://this", "explicit failure")); echo "\n";

$f = fopen("none://this", "r");
var_dump(fclose($f));
var_dump(fclose($f));
$f = fopen("none://this", "r");
var_dump(fflush($f));
var_dump(fflush($f));
var_dump(fclose($f));
echo "\n";

$f = fopen("null://this", "r");
var_dump(fclose($f));
var_dump(fclose($f));
$f = fopen("null://this", "r");
var_dump(fflush($f));
var_dump(fflush($f));
var_dump(fclose($f));
echo "\n";

$f = fopen("yes://this", "r");
var_dump(fclose($f));
var_dump(fclose($f));
$f = fopen("yes://this", "r");
var_dump(fflush($f));
var_dump(fflush($f));
var_dump(fclose($f));
echo "\n";

$f = fopen("no://this", "r");
var_dump(fclose($f));
var_dump(fclose($f));
$f = fopen("no://this", "r");
var_dump(fflush($f));
var_dump(fflush($f));
var_dump(fclose($f));
