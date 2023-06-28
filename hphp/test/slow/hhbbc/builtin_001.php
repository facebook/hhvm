<?hh

function heh() :mixed{ return 'sscanf'; }
function foo(string $s) :mixed{
  $fn = heh();
  list($i) = $fn($s, "%d");
  echo $i;
  echo "\n";
}


<<__EntryPoint>>
function main_builtin_001() :mixed{
foo("12");
}
