<?hh
/*
 * Prototype: bool flock(resource $handle, int $operation [, int &$wouldblock]);
 * Description: PHP supports a portable way of locking complete files
 * in an advisory way
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions ***\n";

$file = sys_get_temp_dir().'/'.'flock.tmp';
$fp = fopen($file, "w");

/* array of operatons */
$operations = vec[
  0,
  LOCK_NB,
  FALSE,
  NULL,
  vec[1,2,3],
  vec[],
  "string",
  "",
  "\0"
];

$i = 0;
foreach($operations as $operation) {
  echo "\n--- Iteration $i ---";
  $wouldblock = false;
  try { var_dump(flock($fp, $operation, inout $wouldblock)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $i++;
}


/* Invalid arguments */
$fp = fopen($file, "w");
fclose($fp);
$wouldblock = false;
var_dump(flock($fp, LOCK_SH|LOCK_NB, inout $wouldblock));

$var = false;
try { var_dump(flock("", "", inout $var)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* No.of args leass than expected */
try { var_dump(flock()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(flock($fp)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* No.of args greater than expected */
try { var_dump(flock($fp, "", inout $var, "")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n*** Done ***\n";
error_reporting(0);
$file = dirname(__FILE__)."/flock.tmp";
unlink($file);
}
