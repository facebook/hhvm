<?hh

function f($a) :mixed{
  var_dump($a);
}
class ClassA {
  public static $val = 1;
  function foo() :mixed{
    $val = 'val'; f($val);
    $this->$val = 2; f($this->$val);
  }
  function foo2() :mixed{
    $this->val = 3; f($this->val);
  }
  function bar() :mixed{
    try {
      var_dump($val);
    } catch (UndefinedVariableException $e) {
      var_dump($e->getMessage());
    }
    var_dump($this->val);
  }
}

<<__EntryPoint>>
function main_1704() :mixed{
  $obj = new ClassA();
  var_dump($obj);
  $obj->foo();
  var_dump($obj);
  $obj->bar();
  $obj->foo2();
  var_dump($obj);
  $obj->bar();
}
