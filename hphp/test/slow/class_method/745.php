<?hh

class AB {
  static function foo() {
    var_dump('foo');
  }
}
function f($x) {
  $a = $x.'B';
  $a::foo();
}

<<__EntryPoint>>
function main_745() {
f('A');
}
