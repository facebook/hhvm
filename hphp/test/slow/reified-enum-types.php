<?hh

newtype newint_t = int;
type int_t = int;

enum Foo : int as int {
  A = 1;
  B = 2;
  C = 3;
}

enum Bar : Foo as int {
  X = Foo::A;
  Y = Foo::B;
}

enum Biz : string {
  A = 'a';
  B = 'b';
  C = 'c';
}

enum Buz : Biz {
  X = Biz::A;
  Y = Biz::B;
}

enum Alpha : arraykey {
  A = 'a';
  B = 2;
  C = 'c';
}

enum Beta : Alpha {
  A = 1;
  B = 'b';
  C = 3;
}

enum Gamma : newint_t {
  A = 1;
  B = 2;
  C = 3;
}

enum Delta : int_t {
  A = 1;
  B = 2;
  C = 3;
}

type Epsilon = Alpha;
newtype Zeta = Beta;

function rfun<reify T, reify R>(T $val): R {
  echo "rfun($val): "; var_dump($val);
  echo "rfun($val): "; var_dump($val is T);
  try {
    $val as T;
    echo "rfun($val): as ok\n";
  } catch (Exception $ex) {
    echo "rfun($val): caught: ".$ex->getMessage()."\n";
  }

  return $val;
}

<<__EntryPoint>>
function main() {
  set_error_handler(function (int $no, string $errmsg, string $f, int $l) {
    throw new Exception($errmsg);
  });

  $v1 = 1; $p1 = __hhvm_intrinsics\launder_value(1);
  $vb = 'b'; $pb = __hhvm_intrinsics\launder_value('b');

  $v3 = 3; $p3 = __hhvm_intrinsics\launder_value(3);
  $vc = 'c'; $pc = __hhvm_intrinsics\launder_value('c');

  $v4 = 4; $p4 = __hhvm_intrinsics\launder_value(4);
  $vo = new stdClass; $po = __hhvm_intrinsics\launder_value(new stdClass);

  rfun<Foo, Foo>($v1);
  rfun<Foo, Foo>($p1);
  rfun<Bar, Bar>($v1);
  rfun<Bar, Bar>($p1);
  rfun<Bar, Bar>($v3);
  rfun<Bar, Bar>($p3);

  rfun<Biz, Biz>($vb);
  rfun<Biz, Biz>($pb);
  rfun<Buz, Buz>($vb);
  rfun<Buz, Buz>($pb);
  rfun<Buz, Buz>($vc);
  rfun<Buz, Buz>($pc);

  rfun<Alpha, Alpha>($vc);
  rfun<Alpha, Alpha>($p1);
  rfun<Beta,  Beta> ($vc);
  rfun<Beta,  Beta> ($p1);
  rfun<Beta,  Beta> ($v3);
  rfun<Beta,  Beta> ($p3);

  rfun<Gamma, Gamma>($v4);
  rfun<Gamma, Gamma>($p4);
  rfun<Delta, Delta>($v1);
  rfun<Delta, Delta>($p1);
  rfun<Delta, Delta>($v4);
  rfun<Delta, Delta>($p4);

  rfun<Epsilon, Epsilon>($v1);
  rfun<Epsilon, Epsilon>($p1);
  rfun<Zeta,    Zeta>   ($vc);
  rfun<Zeta,    Zeta>   ($p1);
  rfun<Zeta,    Zeta>   ($v3);
  rfun<Zeta,    Zeta>   ($p3);

  try {
    rfun<Gamma, Gamma>($pb);
    echo "Error!\n";
  } catch (Exception $ex) {
    echo "ok: ".$ex->getMessage()."\n";
  }
}
