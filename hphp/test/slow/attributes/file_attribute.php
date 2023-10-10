<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<file: MyAttribute('abc123')>>

class MyAttribute implements HH\FileAttribute {
  public function __construct(public string $s)[] { }
}

function reflect(): void {
  $rf = new ReflectionFile(__FILE__);
  $attribute = $rf->getAttributeClass(MyAttribute::class);
  $attribute_value = $attribute->s;
  var_dump($attribute_value);
}
<<__EntryPoint>> function main(): void {
echo reflect();
}
