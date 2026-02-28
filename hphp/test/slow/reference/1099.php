<?hh

<<__DynamicallyCallable>>
function f($arg0, $arg1) :mixed{
  var_dump($arg0, $arg1);
}
<<__DynamicallyCallable>>
function g(inout $arg0, $arg1) :mixed{
  var_dump($arg0, $arg1);
}
class Af {
  function g($f, $var) :mixed{
    $f($this, $var++);
  }
}
class Ag {
  function g($f, $var) :mixed{
    $thiz = $this;
    $f(inout $thiz, $var++);
  }
}

<<__EntryPoint>>
function main_1099() :mixed{
$af = new Af;
$ag = new Ag;
$af->g(f<>, 30);
$ag->g(g<>, 30);
}
