<?hh

<<file: __EnableUnstableFeatures('case_types')>>

final class A {}
final class B {}
final class C {}

case type C1 = int | string;
case type C2 = vec<int> | A;
case type C3 = B | C2;
case type C4 = int | HH\AnyArray<arraykey, mixed>;
case type C5 = int | HH\classname<A>;
case type C6 = int | nonnull;
case type C7 = ?float | nonnull;

class T0 {
  <<__LateInit>> public C1 $a;
  <<__LateInit>> public C2 $b;
  <<__LateInit>> public C3 $c;
  <<__LateInit>> public C4 $d;
  <<__LateInit>> public C5 $e;
  <<__LateInit>> public C6 $f;
  <<__LateInit>> public C7 $g;
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

  $t = new T0();
  $t->a = 5;
  $t->a = "abc";
  expect_exception(
    () ==> {
      $t->a = 5.0;
    },
    "Property 'T0::a' declared as type C1, float assigned",
  );

  $t->b = vec[];
  $t->b = new A();
  expect_exception(
    () ==> {
      $t->b = 5;
    },
    "Property 'T0::b' declared as type C2, int assigned",
  );
  expect_exception(
    () ==> {
      $t->b = "abc";
    },
    "Property 'T0::b' declared as type C2, string assigned",
  );

  $t->c = new B();
  $t->c = vec[];
  $t->c = new A();
  expect_exception(
    () ==> {
      $t->c = 5;
    },
    "Property 'T0::c' declared as type C3, int assigned",
  );

  $t->d = 5;
  $t->d = vec[];
  $t->d = dict[];
  $t->d = keyset[];
  expect_exception(
    () ==> {
      $t->d = new A();
    },
    "Property 'T0::d' declared as type C4, A assigned",
  );

  $t->e = 5;
  $t->e = A::class;
  $t->e = B::class; // classname is erased so this is allowed.
  expect_exception(
    () ==> {
      $t->e = new A();
    },
    "Property 'T0::e' declared as type C5, A assigned",
  );

  $t->f = 5;
  $t->f = "abc";
  $t->f = new A();
  expect_exception(
    () ==> {
      $t->f = null;
    },
    "Property 'T0::f' declared as type C6, null assigned",
  );

  $t->g = 5;
  $t->g = "abc";
  $t->g = null;

  echo "done\n";
}
