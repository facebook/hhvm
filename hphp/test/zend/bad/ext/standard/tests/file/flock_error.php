<?php
/* 
Prototype: bool flock(resource $handle, int $operation [, int &$wouldblock]);
Description: PHP supports a portable way of locking complete files 
  in an advisory way
*/

echo "*** Testing error conditions ***\n";

$file = dirname(__FILE__)."/flock.tmp";
$fp = fopen($file, "w");

/* array of operatons */
$operations = array(
  0,
  LOCK_NB,
  FALSE,
  NULL,
  array(1,2,3),
  array(),
  "string",
  "",
  "\0" 
);

$i = 0;
foreach($operations as $operation) {
  echo "\n--- Iteration $i ---";
  var_dump(flock($fp, $operation));
  $i++;
}


/* Invalid arguments */
$fp = fopen($file, "w");
fclose($fp);
var_dump(flock($fp, LOCK_SH|LOCK_NB));

var_dump(flock("", "", $var));

/* No.of args leass than expected */
var_dump(flock());
var_dump(flock($fp));

/* No.of args greater than expected */
var_dump(flock($fp, "", $var, ""));

echo "\n*** Done ***\n";
?>
<?php
$file = dirname(__FILE__)."/flock.tmp";
unlink($file);
?>