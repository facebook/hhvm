<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class FA implements HH\FunctionAttribute {
  public function __construct(public int $i = 4)[] {}
}

class FAmissing implements HH\FunctionAttribute {
  public function __construct()[] {}
}

class PA implements HH\ParameterAttribute {
  public function __construct()[] {}
}

<<FA>>
function f(<<PA>> int $i) :mixed{}

<<FA(42)>>
function g() :mixed{}

function reflect () :mixed{
  $rf = new ReflectionFunction("f");
  var_dump($rf->getAttributeClass(FA::class)->i); // 4
  $rg = new ReflectionFunction("g");
  var_dump($rg->getAttributeClass(FA::class)->i); // 42
  var_dump(is_null($rf->getAttributeClass(FAmissing::class))); // true
  $rp = $rf->getParameters()[0];
  var_dump(is_null($rp->getAttributeClass(PA::class))); // false
}


<<__EntryPoint>>
function main_empty_attribute_default_arg_vs_missing() :mixed{
echo reflect();
}
