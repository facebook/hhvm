<?hh

function f(string $x, string $y, string $z): string {
  return $x.$y.$z;
}

echo f('a', 'b', 'c');
echo "\n";
