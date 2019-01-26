<?hh

function f() {
  global $y;

  $x = 0;
  $y = 0;
  print ":".empty($x).":\n";
  print ":".empty($y).":\n";

  $x = 1;
  $y = 1;
  print ":".empty($x).":\n";
  print ":".empty($y).":\n";
}
f();
