<?php

function get_rf_for_method($fn, $class) {
  $rc = new ReflectionClass($class);
  return $rc->getMethod($fn);
}

function show($fn, $class=null) {
  $rf = null;
  if ($class) {
    $rf = get_rf_for_method($fn, $class);
  }
 else {
    $rf = new ReflectionFunction($fn);
  }
  $params = $rf->getParameters();
  foreach ($params as $param) {
    echo "{$param->getName()}:\n";
    $attrs = $param->getAttributes();
    ksort($attrs);
    var_dump($attrs);
  }
}

function showr($fn, $class=null) {
  $rf = null;
  if ($class) {
    $rf = get_rf_for_method($fn, $class);
  } else {
    $rf = new ReflectionFunction($fn);
  }
  $params = $rf->getParameters();
  foreach ($params as $param) {
    echo "{$param->getName()}:\n";
    $attrs = $param->getAttributesRecursive();
    ksort($attrs);
    var_dump($attrs);
  }
}

function doboth($fn, $class=null) {
  echo ">>> ";
  if ($class) echo "$class::";
  echo "$fn =>\n---- non-recursive: ----\n";
  show($fn, $class);
  echo "\n---- recursive: ----\n";
  showr($fn, $class);
  echo "\n";
}

//------------------------

function no_attrs($p1, $p2) {
}
function simple_attr(<<Attribute>> $param) {
}
function two_attrs(<<Attr1>> $p1, <<Attr2>> $p2) {
}

class C {
  public static function m(<<Attr(1,2,3)>> $param) {
}
  public static function n(<<Foo, Bar>> $param) {
}
  public function o(<<Hi('bye')>> $param) {
}
  public function p(<<A('b', array('c', 'd')), E('fg')>> $param) {
}
  public function q(<<RS>> $tuv) {
}
  public function wxy(<<And_>> $z, <<NextTime>> $wont_you_sing_with_me) {
}
}

class D extends C {
  // Static functions shouldn't care about the parent class
  public static function m($param) {
}

  // TODO: should we include n, and should m's attrs inherit from C::m?

  // Changing the value of the attribute
  public function o(<<Hi('hello')>> $param) {
}

  // Changing the name of the parameter
  public function q($rstuv) {
}

  // Adding an attribute and leaving one off
  public function wxy(<<EnglishPeopleCallThisZed>> $z, $wont_you_sing_with_me) {
}
}

//------------------------

doboth('no_attrs');
doboth('simple_attr');
doboth('two_attrs');
doboth('m', 'C');
doboth('n', 'C');
doboth('o', 'C');
doboth('p', 'C');
doboth('q', 'C');
doboth('wxy', 'C');
doboth('m', 'D');
doboth('n', 'D');
doboth('o', 'D');
doboth('p', 'D');
doboth('q', 'D');
doboth('wxy', 'D');
