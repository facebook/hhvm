<?hh

function heh() { return 'sscanf'; }
function foo(string $s) {
  $fn = heh();
  $fn($s, "%d", $i);
  echo $i;
  echo "\n";
}

foo("12");
