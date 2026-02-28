<?hh

trait Too {
  function gen() :mixed{
    $abc = $this->input();
    $a = function ($arg) use ($abc) {
      var_dump($arg);
      var_dump($abc);
      return $this->output();
    }
;
    return $a;
  }
  function input() :mixed{
 return 1;
 }
  function output() :mixed{
 return 2;
 }
}
class Foo {
  use Too;
  function input() :mixed{
 return "str1";
 }
  function output() :mixed{
 return "str2";
 }
}
class Goo {
  use Too;
  function input() :mixed{
 return false;
 }
  function output() :mixed{
 return true;
 }
}

<<__EntryPoint>>
function main_2088() :mixed{
$of = new Foo;
$f = $of->gen();
var_dump($f(1000));
$og = new Goo;
$g = $og->gen();
var_dump($g(2000));
}
