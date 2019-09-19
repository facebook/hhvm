<?hh

class X {}

function foo($x, $y) {
  return $x == $y;
}

function baz($r, $s) {
  switch ($r) {
  case $s: echo 'arg '; break;
  default: echo 'def ';
  }
}

function bal($r, $s) {
  if ($r == $s) {
    echo 'arg ';
  } else {
    echo 'def ';
  }
}

<<__EntryPoint>>
function main_entry(): void {
  $x = new X;
  foo($x, $x);


  $x = new X;
  $y = new stdClass;
  baz($x, $y);
  bal($x, $y);

  printf("\n");
}
