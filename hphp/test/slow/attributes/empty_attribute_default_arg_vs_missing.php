<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class __Attribute__FA implements HH\FunctionAttribute {
  public function __construct(public int $i = 4) {}
}

class __Attribute__FAmissing implements HH\FunctionAttribute {
  public function __construct() {}
}

class __Attribute__PA implements HH\ParameterAttribute {
  public function __construct() {}
}

<<FA>>
function f(<<PA>> int $i) {}

<<FA(42)>>
function g() {}

function reflect () {
  $rf = new ReflectionFunction("f");
  var_dump($rf->getAttributeClass(__Attribute__FA::class)->i); // 4
  $rg = new ReflectionFunction("g");
  var_dump($rg->getAttributeClass(__Attribute__FA::class)->i); // 42
  var_dump(is_null($rf->getAttributeClass(__Attribute__FAmissing::class))); // true
  $rp = $rf->getParameters()[0];
  var_dump(is_null($rp->getAttributeClass(__Attribute__PA::class))); // false
}


<<__EntryPoint>>
function main_empty_attribute_default_arg_vs_missing() {
echo reflect();
}
