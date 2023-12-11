<?hh

class A {
  public $a;
  function f() :mixed{
    var_dump($this->a);
  }
  function g() :mixed{
    $this->a = 100;
    call_user_func(vec['self', 'f']);
  }
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
