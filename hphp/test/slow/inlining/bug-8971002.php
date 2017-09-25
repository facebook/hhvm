<?hh // strict

  class A {
    public static function foo($args): void {
      echo "passed\n";
    }
  }

class B {
  public static function main(): void {
    $vector = new Vector (
      fb_call_user_func_safe_return(
        'A::foo',
        Vector{},
        1,
      ),
    );
  }
}

B::main();
