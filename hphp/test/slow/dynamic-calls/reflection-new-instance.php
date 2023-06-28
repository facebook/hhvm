<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class SomeClass {
}

function reflection_construct() :mixed{
  $reflect  = new ReflectionClass(SomeClass::class);
  return $reflect->newInstance();
}

function construct() :mixed{
  $cls = SomeClass::class;
  return new $cls();
}

class ConstructClass {
  public function __construct()[] {}
}

function magic_construct() :mixed{
  $cls = ConstructClass::class;
  return new $cls();
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(reflection_construct());
  var_dump(construct());
  var_dump(magic_construct());
}
