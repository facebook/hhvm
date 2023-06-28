<?hh


<<__EntryPoint>>
function main_badconstruct() :mixed{
try {
  $t = new IntervalTimer(1, function() { echo "ping\n"; });
  echo "Failed\n";
} catch (Exception $e) {
  echo "OK\n";
}
}
