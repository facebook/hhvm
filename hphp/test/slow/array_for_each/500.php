<?hh

class X {
}
function test() :mixed{
  $a = vec[new X, 0];
  foreach ($a as $v) {
    var_dump($v);
  }
  $a = null;
  var_dump('done');
}

<<__EntryPoint>>
function main_500() :mixed{
test();
var_dump('exit');
}
