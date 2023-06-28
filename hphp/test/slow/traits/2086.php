<?hh

trait Too {
  static function bar() :mixed{
    $a = function () {
 var_dump(__CLASS__, __FUNCTION__);
}
;
    $a();
  }
}
class Foo {
 use Too;
 }

<<__EntryPoint>>
function main_2086() :mixed{
Foo::bar();
}
