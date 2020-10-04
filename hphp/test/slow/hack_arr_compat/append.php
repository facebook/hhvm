<?hh

class C {}

<<__EntryPoint>>
function main() {
  $x = dict[];
  $x[] = new C();
  $x['a'] = new C();
  $x[] = new C();
  $x[17] = new C();
  $x[] = new C();
  var_dump($x);
}
