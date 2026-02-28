<?hh

class Foo {}

function handler($_, $str, $file, $line) :mixed{
  if (preg_match('/Object of class (.*) could not be converted to (float|int)/', $str)) {
    throw new TypecastException($str." in $file on line $line");
  }
  return false;
}

function with_exn($fn): mixed {
  try {
    $fn();
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
}

function with_exnio(inout $x, $fn): mixed {
  try {
    $fn(inout $x);
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main(): void {
  set_error_handler(handler<>);
  plus();
  minus();
  mul();
  div();
  mod();
  pow_();
}

function plus(): void {
  echo 'plus<';
  with_exn(() ==> print(0 + -10));
  with_exn(() ==> print(1.234 + INF));
  with_exn(() ==> print(NAN + true));
  with_exn(() ==> print(false + null));
  with_exn(() ==> print(HH\stdin() + "string"));
  with_exn(() ==> print(vec[42] + dict['foobar' => false]));
  with_exn(() ==> print(new Foo() + -INF));

  echo ">\n";

  $i = 1;
  echo 'pluseq<';
  with_exnio(inout $i, (inout $o) ==> { $o += 0; });
  with_exnio(inout $i, (inout $o) ==> { $o += -10; });
  with_exnio(inout $i, (inout $o) ==> { $o += 1.234; });
  with_exnio(inout $i, (inout $o) ==> { $o += INF; });
  with_exnio(inout $i, (inout $o) ==> { $o += NAN; });
  with_exnio(inout $i, (inout $o) ==> { $o += true; });
  with_exnio(inout $i, (inout $o) ==> { $o += false; });
  with_exnio(inout $i, (inout $o) ==> { $o += HH\stdin(); });
  with_exnio(inout $i, (inout $o) ==> { $o += "string"; });
  with_exnio(inout $i, (inout $o) ==> { $o += new Foo(); });
  echo $i;
  echo ">\n";
}

function minus(): void {
  echo 'minus<';
  with_exn(() ==> print(0 - -10));
  with_exn(() ==> print(1.234 - INF));
  with_exn(() ==> print(NAN - true));
  with_exn(() ==> print(false - null));
  with_exn(() ==> print(HH\stdin() - "string"));
  with_exn(() ==> print(vec[42] - dict['foobar' => false]));
  with_exn(() ==> print(new Foo() - -INF));
  echo ">\n";

  $i = 1;
  echo 'minuseq<';
  with_exnio(inout $i, (inout $o) ==> { $o -= 0; });
  with_exnio(inout $i, (inout $o) ==> { $o -= -10; });
  with_exnio(inout $i, (inout $o) ==> { $o -= 1.234; });
  with_exnio(inout $i, (inout $o) ==> { $o -= INF; });
  with_exnio(inout $i, (inout $o) ==> { $o -= NAN; });
  with_exnio(inout $i, (inout $o) ==> { $o -= true; });
  with_exnio(inout $i, (inout $o) ==> { $o -= false; });
  with_exnio(inout $i, (inout $o) ==> { $o -= HH\stdin(); });
  with_exnio(inout $i, (inout $o) ==> { $o -= "string"; });
  with_exnio(inout $i, (inout $o) ==> { $o -= new Foo(); });
  echo $i;
  echo ">\n";
}

function mul(): void {
  echo 'mul<';
  with_exn(() ==> print(0 * -10));
  with_exn(() ==> print(1.234 * INF));
  with_exn(() ==> print(NAN * true));
  with_exn(() ==> print(false * null));
  with_exn(() ==> print(HH\stdin() * "string"));
  with_exn(() ==> print(vec[42] * dict['foobar' => false]));
  with_exn(() ==> print(new Foo() * -INF));
  echo ">\n";

  $i = 1;
  echo "muleq<";
  with_exnio(inout $i, (inout $o) ==> { $o *= 0; });
  with_exnio(inout $i, (inout $o) ==> { $o *= -10; });
  with_exnio(inout $i, (inout $o) ==> { $o *= 1.234; });
  with_exnio(inout $i, (inout $o) ==> { $o *= INF; });
  with_exnio(inout $i, (inout $o) ==> { $o *= NAN; });
  with_exnio(inout $i, (inout $o) ==> { $o *= true; });
  with_exnio(inout $i, (inout $o) ==> { $o *= false; });
  with_exnio(inout $i, (inout $o) ==> { $o *= HH\stdin(); });
  with_exnio(inout $i, (inout $o) ==> { $o *= "string"; });
  with_exnio(inout $i, (inout $o) ==> { $o *= new Foo(); });
  echo $i;
  echo ">\n";
}

function div(): void {
  echo 'div<';
  with_exn(() ==> print(0 / -10));
  with_exn(() ==> print(1.234 / INF));
  with_exn(() ==> print(null / true));
  with_exn(() ==> print(false / NAN));
  with_exn(() ==> print("string" / HH\stdin()));
  with_exn(() ==> print(vec[42] / dict['foobar' => false]));
  with_exn(() ==> print(-INF / new Foo()));
  echo ">\n";

  $i = 1;
  echo "diveq<";
  with_exnio(inout $i, (inout $o) ==> { $o /= -10; });
  with_exnio(inout $i, (inout $o) ==> { $o /= 1.234; });
  with_exnio(inout $i, (inout $o) ==> { $o /= INF; });
  with_exnio(inout $i, (inout $o) ==> { $o /= NAN; });
  with_exnio(inout $i, (inout $o) ==> { $o /= true; });
  with_exnio(inout $i, (inout $o) ==> { $o /= HH\stdin(); });
  with_exnio(inout $i, (inout $o) ==> { $o /= "12string"; });
  with_exnio(inout $i, (inout $o) ==> { $o /= new Foo(); });
  echo $i;
  echo "\n";
}

function mod(): void {
  echo 'mod<';
  with_exn(() ==> print(0 % -10));
  with_exn(() ==> print(null % true));
  with_exn(() ==> print(false % NAN));
  with_exn(() ==> print(HH\stdin() % "12string"));
  with_exn(() ==> print(vec[42] % dict['foobar' => false]));
  with_exn(() ==> print(-INF % new Foo()));
  echo ">\n";

  $i = 1;
  echo "modeq<";
  with_exnio(inout $i, (inout $o) ==> { $o %= -10; });
  with_exnio(inout $i, (inout $o) ==> { $o %= 1.234; });
  with_exnio(inout $i, (inout $o) ==> { $o %= NAN; });
  with_exnio(inout $i, (inout $o) ==> { $o %= true; });
  with_exnio(inout $i, (inout $o) ==> { $o %= HH\stdin(); });
  with_exnio(inout $i, (inout $o) ==> { $o %= "12string"; });
  with_exnio(inout $i, (inout $o) ==> { $o %= vec[42]; });
  with_exnio(inout $i, (inout $o) ==> { $o %= dict['foobar' => false]; });
  with_exnio(inout $i, (inout $o) ==> { $o %= new Foo(); });
  echo $i;
  echo ">\n";
}

function pow_(): void {
  echo 'pow<';
  with_exn(() ==> print(0 ** -10));
  with_exn(() ==> print(1.234 ** INF));
  with_exn(() ==> print(NAN ** true));
  with_exn(() ==> print(false ** null));
  with_exn(() ==> print(HH\stdin() ** "string"));
  with_exn(() ==> print(vec[42] ** dict['foobar' => false]));
  with_exn(() ==> print(new Foo() ** -INF));
  echo ">\n";

  $i = 1;
  echo "poweq<";
  with_exnio(inout $i, (inout $o) ==> { $o **=  0; });
  with_exnio(inout $i, (inout $o) ==> { $o **=  -10; });
  with_exnio(inout $i, (inout $o) ==> { $o **=  1.234; });
  with_exnio(inout $i, (inout $o) ==> { $o **=  INF; });
  with_exnio(inout $i, (inout $o) ==> { $o **=  NAN; });
  with_exnio(inout $i, (inout $o) ==> { $o **=  true; });
  with_exnio(inout $i, (inout $o) ==> { $o **=  false; });
  with_exnio(inout $i, (inout $o) ==> { $o **=  HH\stdin(); });
  with_exnio(inout $i, (inout $o) ==> { $o **=  "string"; });
  with_exnio(inout $i, (inout $o) ==> { $o **=  vec[42]; });
  with_exnio(inout $i, (inout $o) ==> { $o **=  dict['foobar' => false]; });
  with_exnio(inout $i, (inout $o) ==> { $o **= new Foo(); });

  echo $i;
  echo ">\n";
}
