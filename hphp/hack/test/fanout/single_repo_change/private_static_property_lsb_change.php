//// base-parent.php
<?hh

class ParentClass {
  private static int $target = 0;
}

//// base-child.php
<?hh

/**
 * Adding __LSB to the parent's private static property must recheck this child.
 * The child's existing property becomes an illegal override.
 */
class ChildClass extends ParentClass {
  private static int $target = 1;
}

//// changed-parent.php
<?hh

class ParentClass {
  <<__LSB>>
  private static int $target = 0;
}
