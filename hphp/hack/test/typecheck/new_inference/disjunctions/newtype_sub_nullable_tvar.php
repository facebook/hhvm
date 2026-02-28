////file1.php
<?hh

newtype N as ?B = ?B;
newtype M as ?N = ?N;

////file2.php
<?hh

class A<T> {
  private ?T $x;
  public function get(): T {
    if ($this->x === null) { throw new Exception("", 1); }
    return $this->x;
  }
  public function set(?T $x): void {
    $this->x = $x;
  }
}
class B {}

function expect_B(B $_): void {}
function expect_nullB(?B $_): void {}
function expect_N(N $_): void {}
function expect_M(M $_): void {}
function expect_int(int $_): void {}

function test1(M $n): void {
  $x = new A();
  $x->set($n);
  expect_int($x->get()); // error
}

function test2(M $n): void {
  $x = new A(); // $x : A<#0>
  expect_N($x->get()); // #0 <: N
  $x->set($n); /* (M <: #0 || N <: #0 || B <: #0).
  We take the first that works, i.e. N <: #0, and discard the others */
  expect_M($x->get()); // error N incompatible with M
  expect_B($x->get()); // error N incompatible with B
  expect_nullB($x->get()); // ok
}

function test3(M $n): void {
  $x = new A(); // $x : A<#0>
  expect_nullB($x->get()); // #0 <: N
  $x->set($n); /* (M <: #0 || N <: #0 || B <: #0).
  We take the first that works, i.e. M <: #0, and discard the others */
  expect_M($x->get()); // ok
  expect_B($x->get()); // error ?B incompatible with B
}

function test4(M $n): void {
  $x = new A(); // $x : A<#0>
  expect_int($x->get()); // #0 <: N
  $x->set($n); /* (M <: #0 || N <: #0 || B <: #0).
  We take the first that works, i.e. none, so we error. */
}
