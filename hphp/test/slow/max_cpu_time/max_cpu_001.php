<?hh

function main() {
  sleep(3);
}


<<__EntryPoint>>
function main_max_cpu_001() {
ini_set('hhvm.max_cpu_time', 1);
ini_set('max_execution_time', 2);

main();
}
