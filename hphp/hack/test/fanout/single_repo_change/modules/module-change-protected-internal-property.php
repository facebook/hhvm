//// base-modules.php
<?hh
new module A {}
new module B {}

//// base-parent.php
<?hh
module A;

class ParentClass {
  protected internal int $value = 1;
}

//// base-child.php
<?hh
module A;

class ChildClass extends ParentClass {
  public function read_value(): int {
    return $this->value;
  }
}

//// changed-parent.php
<?hh
/**
 * Moving the parent to another module must recheck the child's property access.
 * The inherited protected internal property becomes inaccessible to the child.
 */
module B;

class ParentClass {
  protected internal int $value = 1;
}
