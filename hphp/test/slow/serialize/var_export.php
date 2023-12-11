<?hh

class A {
  protected $b = 'b';
  private $c = 'c';
  public $d = 'd';
  public $e = vec[vec['e']];
}

<<__EntryPoint>>
function main_var_export() :mixed{
var_export(new A);
}
