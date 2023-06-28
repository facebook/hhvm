<?hh

trait Too {
  function bar() :mixed{
    $a = function () {
      var_dump(__CLASS__);
    };
    $a();
    $a = function () {
      var_dump(get_class());
    };
    $a();
    if (isset($this)) {
      $a = function () {
        var_dump(get_class($this));
      };
      $a();
    }
  }
}
class Foo { use Too; }
<<__EntryPoint>> function main(): void {
$f = new Foo;
echo "Between\n";
$f->bar();
}
