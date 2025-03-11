<?hh

class P {}
class C1 extends P {}
class C2 extends P {}

class Wrapper {
  public static function get_class<T>(T $obj): class<T> {
    throw new Exception();
  }

  public static function get_classname<T>(T $obj): classname<T> {
    return get_class($obj);
  }
}

function expect_cn1(classname<C1> $_): void {}
function expect_c1(class<C1> $_): void {}
function expect_cn2(classname<C2> $_): void {}
function expect_c2(class<C2> $_): void {}

function test_get_classname(P $p): void {
  $c = Wrapper::get_classname($p);

  if ($c === C1::class) {
    expect_cn1($c);
    expect_c1($c);
  }

  if ($c === C2::class) {
    expect_cn2($c);
    expect_c2($c);
  }
}

function test_get_class(P $p): void {
  $c = Wrapper::get_class($p);

  if ($c === C1::class) {
    expect_cn1($c);
    expect_c1($c);
  }

  if ($c === C2::class) {
    expect_cn2($c);
    expect_c2($c);
  }
}
