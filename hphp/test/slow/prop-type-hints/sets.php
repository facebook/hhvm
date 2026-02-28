<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public int $p1 = 123;
  public string $p2 = 'abc';
  public string $p3 = '123';
  public int $p4 = 123;

  public static int $s1 = 123;
  public static string $s2 = 'abc';
  public static string $s3 = '123';
  public static int $s4 = 123;
}

function error_boundary($fn) :mixed{
  try {
    $fn();
  } catch (Exception $e) {
    print("Error: ".$e->getMessage()."\n");
  }
}

class B extends A {
  public function test() :mixed{
    $this->p1 = 'abc';
    $this->p2 = HH\Lib\Legacy_FIXME\cast_for_arithmetic($this->p2);
    $this->p2 += 123;
    error_boundary(() ==> $this->p3++);
    $this->p4 += 123;

    self::$s1 = 'abc';
    self::$s2 = HH\Lib\Legacy_FIXME\cast_for_arithmetic(self::$s2);
    self::$s2 += 123;
    error_boundary(() ==> self::$s3++);
    self::$s4 += 123;
  }
}
<<__EntryPoint>> function main(): void {
(new B())->test();
}
