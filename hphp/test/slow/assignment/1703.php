<?hh

function f($a) :mixed{
  var_dump($a);
}
class ClassA {
  public $val;
  function foo() :mixed{
    $val = 1; f($val);
  }
  function bar() :mixed{
    $this->val = 1; f($this->val);
  }
  function goo() :mixed{
    $val = 'val'; f($val);
    $this->$val = 2; f($this->$val);
  }
  function zoo() :mixed{
    try {
      var_dump($val);
    } catch (UndefinedVariableException $e) {
      var_dump($e->getMessage());
    }
    var_dump($this->val);
  }
}
function foo() :mixed{
  $val2 = 1; f($val2);
}

<<__EntryPoint>>
function main() :mixed{
  $obj = new ClassA();
  var_dump($obj);
  $obj->foo();
  var_dump($obj);
  $obj->bar();
  var_dump($obj);
  $obj->goo();
  var_dump($obj);
  $obj->zoo();
}
