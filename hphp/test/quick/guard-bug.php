<?hh

abstract final class GuardBug { public static $x; }

function k() :mixed{
  // a comment!
  if (!isset(GuardBug::$x)) {
    GuardBug::$x = fopen(__FILE__, 'r');
  }
  return GuardBug::$x;
}

<<__EntryPoint>>
function main(): void {
  while ($line = fgets(k())) {
      $parts = explode(' ', $line);
      $file = $parts[0];

      echo "$file -\n";
  }
}
