<?hh

function heh() { return 'sscanf'; }
function foo(string $s) {
  $fn = heh();
  $fn($s, "%d", &$i);
  echo $i;
  echo "\n";
}


<<__EntryPoint>>
function main_builtin_001() {
foo("12");
}
