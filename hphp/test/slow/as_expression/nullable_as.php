<?hh

function f(mixed $x) {
  $y = $x ?as int;
  if ($y === null) {
    echo 'not int';
  } else {
    echo 'int';
  }
  echo "\n";
}

f(1);
f("hi");
f(1.20);
f(true);
