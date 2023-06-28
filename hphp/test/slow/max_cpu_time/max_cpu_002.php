<?hh


<<__EntryPoint>>
function main_max_cpu_002() :mixed{
ini_set('hhvm.max_cpu_time', 1);
ini_set('max_execution_time', 20);
sleep(1);
echo "ok\n";
$now = time();
while (time() - $now < 20) {
  // busy wait
}
}
