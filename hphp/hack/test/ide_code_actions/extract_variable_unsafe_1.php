<?hh

class A {
  public function __construct(public bool $b) {}
  public function toggle_mut_and_ret_false(): bool {
    $this->b = !$this->b;
    return false;
  }
}

<<__EntryPoint>>
function main(): void {
  $a = new A(false);
  $x = $a->b || /*range-start*/$a->toggle_mut_and_ret_false()/*range-end*/;
  var_dump($x); // false

  // The refactor can change the meaning of the code in a way that is hard to detect.
  // IntelliJ Kotlin has the same unsafety:  https://pl.kotl.in/isarlHogA
  // $a = new A(false);
  // $placeholder_ = $a->toggle_mut_and_ret_false();
  // $x = $a->b || $placeholder_;
  // var_dump($x); // true
}
