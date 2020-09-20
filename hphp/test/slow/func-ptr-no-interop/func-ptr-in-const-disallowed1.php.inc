<?hh

function foo() {}

class A {
  public static $arr = dict[
    'foo' => fun('foo'),
  ];
}
