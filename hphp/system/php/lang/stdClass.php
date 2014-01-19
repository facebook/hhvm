<?php

// default base
class stdClass {
}

// used in unserialize() for unknown classes
class __PHP_Incomplete_Class {
  public $__PHP_Incomplete_Class_Name;
}

// Used in serialize() for classes which don't support
// serialization. Typically CPP Extension classes.
class __PHP_Unserializable_Class {
  public $__PHP_Unserializable_Class_Name;
}
