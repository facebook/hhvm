<?hh // strict

namespace NS_Stack;

class StackUnderflowException extends \Exception {}

class Stack<T> {
  private array<T> $stack;
  private int $stackPtr;

  public function __construct() {
    $this->stackPtr = 0;
    $this->stack = array();

    echo "Inside " . __METHOD__ . ": stackPtr = " . $this->stackPtr . "\n";
  }

  public function __destruct() {
    $this->stack = array();
  }

  public function push(T $value): void {
    echo "Inside " . __METHOD__ . ": stackPtr = " . $this->stackPtr . "\n";

    $this->stack[$this->stackPtr++] = $value;
  }

  public function pop(): T {
    echo "Inside " . __METHOD__ . ": stackPtr = " . $this->stackPtr . "\n";

    if ($this->stackPtr > 0) {
      return $this->stack[--$this->stackPtr];
    } else {
      throw new StackUnderflowException();
    }
  }

  public function getStackDepth(): int {
    return $this->stackPtr;
  }
}
