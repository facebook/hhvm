<?hh

class A {
  public $a;
  function f() {
    var_dump($this->a);
  }
  function g() {
    $this->a = 100;
    call_user_func(array('self', 'f'));
  }
}
<<__EntryPoint>> function main(): void {}
