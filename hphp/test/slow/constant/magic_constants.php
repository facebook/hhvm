<?php
namespace NS;

const WHERE_I_AM = __FILE__.':'.__LINE__;

class Foo {
  const DOUBLE_THE_FUN = __NAMESPACE__.'\\'.__CLASS__.'!!';
  const INVALIDS = __METHOD__.':'.__FUNCTION__.':'.__TRAIT__.':'.__METHOD__;
}

var_dump(WHERE_I_AM);
var_dump(Foo::DOUBLE_THE_FUN);
var_dump(Foo::INVALIDS);
