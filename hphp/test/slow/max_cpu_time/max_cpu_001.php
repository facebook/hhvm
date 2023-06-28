<?hh

function main() :mixed{
  sleep(3);
}


<<__EntryPoint>>
function main_max_cpu_001() :mixed{
ini_set('hhvm.max_cpu_time', 1);
ini_set('max_execution_time', 2);

main();
}
