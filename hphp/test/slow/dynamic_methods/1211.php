<?hh

function bar() {
  echo 'bar called';
}
class foo {
  public $functions = varray[];
  function __construct() {
    $function = 'bar';
    print($function);
    print($function());
    $this->functions['test'] = $function;
    print($this->functions['test']());
  }
}

<<__EntryPoint>>
function main_1211() {
$a = new foo ();
}
