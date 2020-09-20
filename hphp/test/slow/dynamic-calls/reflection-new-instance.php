<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class SomeClass {
}

function reflection_construct() {
  $reflect  = new ReflectionClass(SomeClass::class);
  return $reflect->newInstance();
}

function construct() {
  $cls = SomeClass::class;
  return new $cls();
}

class ConstructClass {
  public function __construct() {}
}

function magic_construct() {
  $cls = ConstructClass::class;
  return new $cls();
}

<<__EntryPoint>>
function main() {
  var_dump(reflection_construct());
  var_dump(construct());
  var_dump(magic_construct());
}
