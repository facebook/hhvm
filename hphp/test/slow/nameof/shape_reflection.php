<?hh

class C { const string X = "FOO"; }
function f(mixed $m = shape(C::class => 4)): void {
  var_dump($m);
}
function g(mixed $m = shape(nameof C => 5)): void {
  var_dump($m);
}
function h(mixed $m = shape(C::X => 6)): void {
  var_dump($m);
}


function reflect(string $s): void {
  $f = new ReflectionFunction($s);
  $p = $f->getParameters()[0];
  var_dump($p->getDefaultValue());
}

<<__EntryPoint>>
function main(): void {
  reflect('f'); // broken T204024412
  reflect('g');
  reflect('h'); // broken T204024412
}
