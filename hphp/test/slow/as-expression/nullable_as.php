<?hh

function f(mixed $x) :mixed{
  $y = $x ?as int;
  if ($y === null) {
    echo 'not int';
  } else {
    echo 'int';
  }
  echo "\n";
}


<<__EntryPoint>>
function main_nullable_as() :mixed{
f(1);
f("hi");
f(1.20);
f(true);
}
