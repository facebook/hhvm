<?hh

function f(): void {
  $d = dict['a' => 42];
  $e = $d;
  $k = 'b';
  $e['c'] = 'apple';
  $d[$k] = 3.14; // This should cause $e to be invalidated as well.
  inspect($e);
  inspect($d);
}
