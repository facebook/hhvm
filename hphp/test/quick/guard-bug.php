<?hh

function k() {
  global $x;
  if (!isset($x)) {
    $x = fopen(__FILE__, 'r');
  }
  return $x;
}

while ($line = fgets(k())) {
    $parts = explode(' ', $line);
    $file = $parts[0];

    echo "$file -\n";
}
