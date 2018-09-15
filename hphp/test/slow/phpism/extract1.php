<?hh

function f(): void {
  $a = darray[
    'x' => 1,
    'y' => 'bar',
  ];
  extract(&$a);
  var_dump($x);
  var_dump($y);
}

f();
