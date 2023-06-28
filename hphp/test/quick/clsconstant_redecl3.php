<?hh

interface A {
  const FOO = 'FOO';
}

abstract class B implements A {
  const BAR = 'BAR';

  abstract public function test():mixed;
}

class C extends B {
  // This is actually fine. It wouldn't be if C directly implemented A
  const FOO = 'DOH';

  public function test() :mixed{ echo self::FOO . "\n"; }
}
<<__EntryPoint>> function main(): void {
var_dump(C::FOO);
}
