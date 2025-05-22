<?hh

function foo(): void {
  $_ =  function<T>(T $x):T use() { return $x; };
}
