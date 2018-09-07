<?hh

function f(): void {
  $x = 1;
  $y = 'bar';
  var_dump(compact('x', 'y'));
}

f();
