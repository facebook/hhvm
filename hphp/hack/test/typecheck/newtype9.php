//// _comments
<?hh

// opaque type should not implicitly convert to string.
// even when the type backing it does implicit convert to
// a string ordinarily

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
  public string $foo;
  public function __construct() {
    $this->foo = 'B' . (new Opaque())->opaque_int;
  }
};
