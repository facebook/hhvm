<?hh

class WWWTest {}

abstract class AbstractParent {
  <<__TestsBypassVisibility>>
  abstract protected function abs_prot(): int;
}

class ConcreteChild extends AbstractParent {
  <<__Override, __TestsBypassVisibility>>
  protected function abs_prot(): int { return 42; }
}

class AbstractTest extends WWWTest {
  public function test(ConcreteChild $obj): void {
    $_ = $obj->abs_prot(); // ok: both parent and child have the attribute
  }
}

// Missing attribute on the implementation is an error because HHVM checks the
// concrete method at runtime and would fatal if the attribute is absent.
class ConcreteChild2 extends AbstractParent {
  <<__Override>>
  protected function abs_prot(): int { return 99; }
}

class AbstractTest2 extends WWWTest {
  public function test(ConcreteChild2 $obj): void {
    $_ = $obj->abs_prot(); // error: child's override doesn't have the attribute
  }
}

// Even when the receiver is typed as AbstractParent, the runtime dispatches to
// the concrete method. Without the attribute there, HHVM fatals.
class AbstractTest3 extends WWWTest {
  public function test(AbstractParent $obj): void {
    $_ = $obj->abs_prot(); // ok: abstract decl has the attribute
  }
}
