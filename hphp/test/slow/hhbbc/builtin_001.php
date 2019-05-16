<?hh

function heh() { return 'sscanf'; }
function foo(string $s) {
  $fn = heh();
  list($i) = $fn($s, "%d");
  echo $i;
  echo "\n";
}


<<__EntryPoint>>
function main_builtin_001() {
foo("12");
}
