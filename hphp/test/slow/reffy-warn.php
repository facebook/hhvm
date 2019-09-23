<?hh

class Foo {
  function alpha($one, &$two, $three, &$four) {}
}

class Bar extends Foo {
  function alpha(&$one, $two, $three, &$four) {}
}

