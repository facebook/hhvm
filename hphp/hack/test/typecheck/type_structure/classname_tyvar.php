<?hh  

abstract class A {
  abstract const type T;
  abstract const type T2 as A;
}

function expect_string(string $s): void {}
class Test {
  private static function foo(
    classname<A> $kls,
    dynamic $d,
  ): void {
    expect_string(type_structure(HH\classname_to_class($kls), 'T')['classname']);
    expect_string(HH\type_structure_classname(HH\classname_to_class($kls), 'T'));

    expect_string(type_structure(HH\classname_to_class($kls), 'T2')['classname']);
    expect_string(HH\type_structure_classname(HH\classname_to_class($kls), 'T2'));

    expect_string(type_structure(HH\classname_to_class($d), 'T')['classname']);
    expect_string(HH\type_structure_classname(HH\classname_to_class($d), 'T'));
  }
}
