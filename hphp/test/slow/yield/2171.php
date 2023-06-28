<?hh
<<__DynamicallyCallable>>
function f($a1, $a2, $a3) :mixed{
  var_dump($a1, $a2, $a3);
}
<<__DynamicallyCallable>>
function g($a1, $a2, $a3) :mixed{
  var_dump($a1, $a2, $a3);
}
function h($fcn) :AsyncGenerator<mixed,mixed,void>{
  $fcn(\HH\global_get('_POST'), Yield2171::$x, Yield2171::$x++);
  yield 64;
}

abstract final class Yield2171 {
  public static $x = 32;
}
<<__EntryPoint>>
function entrypoint_2171(): void {
  foreach (h(rand(0, 1) ? 'f' : 'g') as $v) {
    var_dump($v);
  }
}
