<?hh

function k() {
  // a comment!
  if (!isset(GuardBug::$x)) {
    GuardBug::$x = fopen(__FILE__, 'r');
  }
  return GuardBug::$x;
}

while ($line = fgets(k())) {
    $parts = explode(' ', $line);
    $file = $parts[0];

    echo "$file -\n";
}
abstract final class GuardBug { public static $x; }
