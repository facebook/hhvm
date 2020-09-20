<?hh

class C {
  const (int, string) FOO = tuple(42, 'foo');
}

<<__EntryPoint>>
function f(): void {
  list($int, $string) = C::FOO;
  var_dump($int);
  var_dump($string);
}
