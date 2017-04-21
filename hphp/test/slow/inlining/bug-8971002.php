<?hh // strict

  class A {
    public static function foo($args) {
      echo "passed\n";
    }
  }

class B {
  public static function main() {
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
