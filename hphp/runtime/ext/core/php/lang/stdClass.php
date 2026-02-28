<?hh

// empty class that's always available
final class stdClass {
}

// used in unserialize() for unknown classes
final class __PHP_Incomplete_Class {
  public $__PHP_Incomplete_Class_Name;

  public function __construct() {
    throw new Exception(
      "Only the unserializer may construct instances of __PHP_Incomplete_Class"
    );
  }
}
