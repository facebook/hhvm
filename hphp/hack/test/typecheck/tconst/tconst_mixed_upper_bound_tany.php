<?hh

// Test that type constant propagation to mixed upper bounds does not
// produce Tany. When a type variable with type constants gains a
// mixed or supportdyn<mixed> upper bound, expanding the type constant
// on mixed used to return Missing and create Tany, which polluted
// the solution of all related type variables.

abstract class Renderer {}

abstract class Notif {
  abstract const type TRenderer as Renderer;
}

abstract final class Utils {
  public static function get<TN as Notif, TR>(TN $_): TR
  /* HH_FIXME[2129] */
    where TR = TN::TRenderer {
    throw new Exception();
  }

  public static async function gen<TN as Notif, TR>(
    TN $_,
  ): Awaitable<TR>
  /* HH_FIXME[2129] */
    where TR = TN::TRenderer {
    throw new Exception();
  }
}

function expect_int(int $_): void {}

function my_map<TV1, TV2>(
  Traversable<TV1> $_traversable,
  (function(TV1): TV2) $_value_func,
): vec<TV2> {
  throw new Exception();
}

// Sync call inside sync lambda: type constant should be preserved.
function test_sync_lambda(Traversable<Notif> $xs): void {
  my_map(
    $xs,
    $n ==> {
      $r = Utils::get($n);
      // With the Tany bug, this would not error because Tany is
      // compatible with int. With the fix, TRenderer is preserved
      // and correctly errors.
      expect_int($r);
      return 1;
    },
  );
}

// Async call inside async lambda: this was the original bug scenario.
// The Awaitable wrapper causes a supportdyn<mixed> upper bound on the
// type variable, which triggered the Tany bug.
function test_async_lambda(Traversable<Notif> $xs): void {
  my_map(
    $xs,
    async $n ==> {
      $r = await Utils::gen($n);
      expect_int($r);
      return 1;
    },
  );
}

// Sync call inside async lambda.
function test_sync_in_async_lambda(Traversable<Notif> $xs): void {
  my_map(
    $xs,
    async $n ==> {
      $r = Utils::get($n);
      expect_int($r);
      return 1;
    },
  );
}

// Direct calls outside lambdas (sanity check).
async function test_direct(Notif $n): Awaitable<void> {
  $r1 = Utils::get($n);
  expect_int($r1);

  $r2 = await Utils::gen($n);
  expect_int($r2);
}
