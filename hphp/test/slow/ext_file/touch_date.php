<?hh

function inRange($x, $a, $b) :mixed{
  return ($x >= $a && $x <= $b) ? "YES" : "NO";
}


<<__EntryPoint>>
function main_touch_date() :mixed{
$file = tempnam(sys_get_temp_dir(), 'touch_date');

// No args
$now = time();
touch($file);
$fileInfo = new SplFileInfo($file);
print(inRange($fileInfo->getMTime(), $now, $now + 10)."\n");
print(inRange($fileInfo->getATime(), $now, $now + 10)."\n");


// Mofification time only
touch($file, strtotime("@100200300"));
$fileInfo = new SplFileInfo($file);
print($fileInfo->getMTime()."\n");
print($fileInfo->getATime()."\n");


// Modification and access time
touch($file, strtotime("@100200300"), strtotime("@100400500"));
$fileInfo = new SplFileInfo($file);
print($fileInfo->getMTime()."\n");
print($fileInfo->getATime()."\n");

unlink($file);
}
