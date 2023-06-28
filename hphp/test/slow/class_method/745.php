<?hh

class AB {
  static function foo() :mixed{
    var_dump('foo');
  }
}
function f($x) :mixed{
  $a = $x.'B';
  $a::foo();
}

<<__EntryPoint>>
function main_745() :mixed{
f('A');
}
