//// base-modules.php
<?hh
new module A {}
new module B {}

//// base-c.php
<?hh
module A;

class C {
  internal function __construct() {}
}

//// base-caller.php
<?hh
module A;

function make_c(): C {
  return new C();
}

//// changed-c.php
<?hh
/**
 * Moving C to another module must recheck the unchanged call to new C().
 * Its internal constructor becomes inaccessible to the caller in module A.
 */
module B;

class C {
  internal function __construct() {}
}
