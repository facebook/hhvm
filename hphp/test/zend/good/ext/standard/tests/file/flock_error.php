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
  try { var_dump(flock($fp, $operation)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $i++;
}


/* Invalid arguments */
$fp = fopen($file, "w");
fclose($fp);
var_dump(flock($fp, LOCK_SH|LOCK_NB));

try { var_dump(flock("", "", &$var)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* No.of args leass than expected */
try { var_dump(flock()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(flock($fp)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* No.of args greater than expected */
try { var_dump(flock($fp, "", &$var, "")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n*** Done ***\n";
?>
<?php error_reporting(0); ?>
<?php
$file = dirname(__FILE__)."/flock.tmp";
unlink($file);
?>
