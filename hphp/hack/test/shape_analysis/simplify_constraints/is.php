<?hh

class C {}

function f(mixed $m): void {
  $d = dict[];
  if ($m is int)  {
    $d['k'] = 42;
  }
  inspect($d);
}
