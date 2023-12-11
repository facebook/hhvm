<?hh

function decieve_static_analysis() :mixed{
  return mt_rand() ? 'a' : 'b';
}

function main() :mixed{
  $start_usage = memory_get_usage();
  $a = vec[];
  for ($i = 0; $i < $start_usage / 1000; $i++) {
    $a[] = str_repeat(decieve_static_analysis(), 1000);
  }
  var_dump(memory_get_usage() > $start_usage);
}

<<__EntryPoint>>
function main_memory_get_usage() :mixed{
main();
}
