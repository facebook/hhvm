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
    $t = $var; $var++; $f($this, $t);
  }
}
class Ag {
  function g($f, $var) :mixed{
    $thiz = $this;
    $t = $var; $var++; $f(inout $thiz, $t);
  }
}

<<__EntryPoint>>
function main_1099() :mixed{
$af = new Af;
$ag = new Ag;
$af->g(f<>, 30);
$ag->g(g<>, 30);
}
