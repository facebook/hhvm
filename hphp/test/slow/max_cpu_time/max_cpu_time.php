<?hh


<<__EntryPoint>>
function main_max_cpu_time() :mixed{
var_dump(ini_get('hhvm.max_cpu_time'));
ini_set('hhvm.max_cpu_time', 1);
var_dump(ini_get('hhvm.max_cpu_time'));
$now = time();
while (time() - $now < 4) {
  // busy wait, should time out.
}
}
