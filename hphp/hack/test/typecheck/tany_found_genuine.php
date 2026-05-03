<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

// Test that the Tany_found warning (Warn[12036]) DOES fire for genuine Tany,
// including when the type flows through unions, intersections, join points,
// and other flows.

interface IWithoutTB {}

abstract class DefinesTB implements IWithoutTB {
  abstract const type TB;
}

abstract class UsesTB {
  abstract const type TA as DefinesTB;
  const type TB = this::TA::TB;
  public function __construct(protected this::TB $tb): void {}
}

// Redeclares TA with an upper bound that doesn't define TB, causing
// this::TB to resolve to genuine Tany
trait TRedeclareTA {
  abstract const type TA as IWithoutTB;
}

trait TGenuineTanyTests {
  require extends UsesTB;
  use TRedeclareTA;

  // ── Direct: genuine Tany from unresolvable type constant ───────────────
  public function test_genuine_direct(): void {
    // $this->tb has type this::TB which resolves to Tany because TA's
    // upper bound (IWithoutTB) doesn't define TB
    // Warn[12036] expected
    $_ = $this->tb;
  }

  // ── Assignment: genuine Tany through assignment ────────────────────────
  public function test_genuine_assignment(): void {
    $a = $this->tb;
    $b = $a;
    // Warn[12036] expected
    $_ = $b;
  }

  // ── Union: genuine Tany in a union ─────────────────────────────────────
  public function test_genuine_union(bool $b, int $y): void {
    $z = $b ? $this->tb : $y;
    // Warn[12036] expected
    $_ = $z;
  }

  // ── Intersection: genuine Tany through `is` refinement ────────────────
  public function test_genuine_intersection(): void {
    $x = $this->tb;
    if ($x is int) {
      // Warn[12036] expected
      $_ = $x;
    }
  }

  // ── Join point: genuine Tany across branches ──────────────────────────
  public function test_genuine_join_point(bool $b): void {
    if ($b) {
      $z = $this->tb;
    } else {
      $z = 42;
    }
    // Warn[12036] expected
    $_ = $z;
  }

  // ── Method call: genuine Tany through method call ─────────────────────
  public function test_genuine_method_call(): void {
    $result = $this->tb->someMethod();
    // Warn[12036] expected
    $_ = $result;
  }

  // ── Shape access: genuine Tany through array access ───────────────────
  public function test_genuine_shape_access(): void {
    $val = $this->tb['key'];
    // Warn[12036] expected
    $_ = $val;
  }

  // ── Chained: genuine Tany through chained method calls ────────────────
  public function test_genuine_chained(): void {
    $result = $this->tb->first()->second();
    // Warn[12036] expected
    $_ = $result;
  }
}
