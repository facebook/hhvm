<?hh

class Foo {
  static function bar() :mixed{
    $a = function () {
 var_dump(__CLASS__, __FUNCTION__);
}
;
    $a();
  }
}

<<__EntryPoint>>
function main_1928() :mixed{
  Foo::bar();
}
