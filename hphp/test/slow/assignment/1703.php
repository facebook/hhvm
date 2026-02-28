<?hh

function f($a) :mixed{
  var_dump($a);
}
class ClassA {
  public $val;
  function foo() :mixed{
    f($val = 1);
  }
  function bar() :mixed{
    f($this->val = 1);
  }
  function goo() :mixed{
    f($val = 'val');
    f($this->$val = 2);
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
  f($val2 = 1);
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
