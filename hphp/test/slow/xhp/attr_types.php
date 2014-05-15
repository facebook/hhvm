<?php

class :base {
  protected static function __xhpAttributeDeclaration() {
    return [];
  }
}

class :testclass extends :base {
  attribute
    callable some_callable,
    float some_float,
    enum {'herp', 'derp'} some_enum,
    var some_var,
    mixed some_mixed,
    Object some_object,
    array some_array,
    int some_int,
    bool some_bool,
    string some_string;

  public static function attrs() {
    return self::__xhpAttributeDeclaration();
  }
}

/*
 * This dumps the type numbers; values taken from
 * https://github.com/facebook/xhp/commit/177b52dddc03
 */
var_dump(array_map($x ==> $x[0], :testclass::attrs()));
