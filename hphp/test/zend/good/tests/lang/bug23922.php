<?hh
class foo
{
  public $foo = 1;

  function as_string()
  { assert('$this->foo == 1'); }

  function as_expr()
  { assert($this->foo == 1); }
}
<<__EntryPoint>> function main(): void {
$foo = new foo();
$foo->as_expr();
$foo->as_string();
}
