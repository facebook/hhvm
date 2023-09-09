<?hh

<<file: __EnableUnstableFeatures('case_types')>>

case type C1 = int | string;
case type C2 = int | string | float;

class A {
  <<__LateInit>> public C1 $a;
}

class B extends A {
  <<__LateInit>> public C2 $a;
}

function expect_exception((function(): void) $f, string $msg): void {
  try {
    $f();
  } catch (Exception $e) {
    $got = $e->getMessage();
    if ($msg !== $got) {
      throw new Exception("Exception caught but mismatched message:\nGot:      $got\nExpected: $msg");
    }
    return;
  }
  throw new Exception("Exception expected but not thrown");
}

<<__EntryPoint>>
function main(): void {
  set_error_handler(
    ($_, string $what, $_, $_, $_) ==> {
      throw new Exception($what);
    },
  );

  $b = new B();

  echo "done\n";
}
