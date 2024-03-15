<?hh

abstract class MyClass {
  abstract public static function foo(): Awaitable<void>;
}

class MyChild extends MyClass {
                     // ^ at-caret
}
