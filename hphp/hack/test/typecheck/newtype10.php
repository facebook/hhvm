//// _comments
<?hh

// Instances of an opaque type should be comparable using === .
// Some languages have a notion of types that don't permit
// checking for sameness (e.g. Haskell for anything *not* implementing Eq).
// Hacklang guarantees that checking sameness is always possible.

//// newtype.php
<?hh

newtype OpaqueInt = int;

class Opaque {
  public OpaqueInt $opaque_int;
  public function __construct() {
    $this->opaque_int = 5;
  }
}

//// useit.php
<?hh

class Klass {
  public bool $cmp;
  public function __construct() {
    $this->cmp = (new Opaque())->opaque_int === (new Opaque())->opaque_int;
  }
};
