<?hh

class C {}

<<__EntryPoint>>
function main() :mixed{
  $x = dict[];
  $x[] = new C();
  $x['a'] = new C();
  $x[] = new C();
  $x[17] = new C();
  $x[] = new C();
  var_dump($x);

  $x = dict[];
  $x[] = new C();
  $x[] = new C();
  $x[] = new C();
  unset($x[1]);
  unset($x[2]);
  $x[] = new C();
  var_dump($x);
}
