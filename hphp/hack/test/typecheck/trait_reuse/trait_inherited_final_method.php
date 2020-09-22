<?hh

trait One {
  require extends MyClass;
}

class MyClass {
  final public function get(): int {
    return 42;
  }
}

class MyParent extends MyClass {
  use One;
}
class MyChild extends MyParent {
  use One;
}

<<__EntryPoint>>
function my_main(): void {
  new MyChild();
}
