<?hh
<<__DynamicallyCallable>>
function bar() :mixed{
  echo 'bar called';
}
class foo {
  public $functions = darray[];
  function __construct() {
    $function = 'bar';
    print($function);
    print($function());
    $this->functions['test'] = $function;
    print($this->functions['test']());
  }
}

<<__EntryPoint>>
function main_1211() :mixed{
$a = new foo ();
}
