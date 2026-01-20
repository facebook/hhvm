# Generic Types

Hack contains a mechanism to define generic&mdash;that is, type-less&mdash;classes, interfaces, and traits, and to create type-specific instances of
them via type parameters.

Consider the following example in which `Stack` is a generic class having one type parameter, `T`, and that implements a stack:

```hack file:stack.hack
class StackUnderflowException extends Exception {}

class Stack<T> {
  private vec<T> $stack;
  private int $stackPtr;

  public function __construct() {
    $this->stackPtr = 0;
    $this->stack = vec[];
  }

  public function push(T $value): void {
    $this->stack[] = $value;
    $this->stackPtr++;
  }

  public function pop(): T {
    if ($this->stackPtr > 0) {
      $this->stackPtr--;
      return $this->stack[$this->stackPtr];
    } else {
      throw new StackUnderflowException();
    }
  }
}
```

As shown, the type parameter `T` (specified in `Stack<T>`) is used as a placeholder in the declaration of the instance property `$stack`, as
the parameter type of the instance method `push`, and as the return type of the instance method `pop`. Note that although `push` and `pop` use
the type parameter, they are not themselves generic methods.

```hack file:stack.hack
function use_int_stack(Stack<int> $stInt): void {
  $stInt->push(10);
  $stInt->push(20);
  $stInt->push(30);
  echo 'pop => '.$stInt->pop()."\n";
}
```

Assuming we construct an instance for a `Stack` of `int`s, we can pass that to function `use_int_stack`, which can push values onto, and pop
values off, that stack, as shown. And in the same or different programs, we can have stacks of `string`, stacks of `?(int, float)`, stacks of
`Employee`, and so on, without have to write type-specific code for each kind of stack.

For more information, see [generic types overview](/hack/generics/introduction).
