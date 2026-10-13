//// base-modules.php
<?hh
new module A {}
new module B {}

//// base-parent.php
<?hh
module A;

class ParentClass {
  protected internal function value(): int {
    return 1;
  }
}

//// base-child.php
<?hh
module A;

class ChildClass extends ParentClass {
  public function read_value(): int {
    return $this->value();
  }
}

//// changed-parent.php
<?hh
/**
 * Moving the parent to another module must recheck the child's method call.
 * The inherited protected internal method becomes inaccessible to the child.
 */
module B;

class ParentClass {
  protected internal function value(): int {
    return 1;
  }
}
