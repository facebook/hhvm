<?php

$badblobs = array(
'x:i:2;i:0;,i:1;;i:0;,i:2;;m:a:0:{}',
'x:i:3;O:8:"stdClass":0:{},O:8:"stdClass":0:{};R:2;,i:1;;O:8:"stdClass":0:{},r:2;;m:a:0:{}',
'x:i:3;O:8:"stdClass":0:{},O:8:"stdClass":0:{};r:2;,i:1;;O:8:"stdClass":0:{},r:2;;m:a:0:{}',
'x:i:1;O:8:"stdClass":0:{},N;;m:s:40:"1234567890123456789012345678901234567890"',
);
foreach($badblobs as $blob) {
try {
  $so = new SplObjectStorage();
  $so->unserialize($blob);
  var_dump($so);
} catch(UnexpectedValueException $e) {
	echo $e->getMessage()."\n";
}
}
echo "DONE\n";
