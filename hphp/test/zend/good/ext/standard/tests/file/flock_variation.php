<?php
/* 
Prototype: bool flock(resource $handle, int $operation [, int &$wouldblock]);
Description: PHP supports a portable way of locking complete files 
  in an advisory way
*/

echo "*** Testing flock() fun with the various operation and 
            wouldblock values                                ***\n";
$file = dirname(__FILE__)."/flock.tmp";
$fp = fopen($file, "w");

/* array of operatons */
$operations = array(
  LOCK_SH,
  LOCK_EX,
  LOCK_SH|LOCK_NB,
  LOCK_EX|LOCK_NB,
  LOCK_SH|LOCK_EX,
  LOCK_UN,
  1, 
  2,
  2.234,
  TRUE
);

/* array of wouldblocks */
$wouldblocks = array(
  0,
  1,
  2,
  1.234,
  TRUE,
  FALSE,
  NULL,
  array(1,2,3),
  array(),
  "string",
  "",
  /* binary input */
  b"string",
  b"",
  "\0"
);

$i = 0;
foreach($operations as $operation) {
  echo "--- Outer iteration $i ---\n";
  var_dump(flock($fp, $operation));
  $j = 0;
  foreach($wouldblocks as $wouldblock) {
    echo "-- Inner iteration $j in $i --\n";
    var_dump(flock($fp, $operation, $wouldblock));
    $j++;
  }
  $i++;
}

fclose($fp);
@unlink($file);

echo "\n*** Done ***\n";
?>