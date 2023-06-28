<?hh
class foo {
  function foo($n=0) :mixed{
    if ($n) throw new Exception("new");
  }
}

<<__EntryPoint>> function main(): void {
$x = new foo();
try {
  $y = $x->foo(1);
} catch (Exception $e) {
  var_dump($x);
}
}
