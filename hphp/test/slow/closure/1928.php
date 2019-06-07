<?hh

class Foo {
  static function bar() {
    $a = function () {
 var_dump(__CLASS__, __FUNCTION__);
}
;
    $a();
  }
}

<<__EntryPoint>>
function main_1928() {
  Foo::bar();
}
