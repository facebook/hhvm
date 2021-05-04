//// base-a.php
<?hh
class A {
  const type T = arraykey;
}
//// base-b.php
<?hh
class B extends A {
  const type  T = arraykey;
}
//// base-c.php
<?hh
class C extends B {}
//// base-use.php
<?hh
function accept_a_const(A::T $_): void {}
function accept_b_const(B::T $_): void {}
function accept_c_const(C::T $_): void {}

function use_a_const(): void { accept_a_const(3); }
function use_b_const(): void { accept_b_const(3); }
function use_c_const(): void { accept_c_const(3); }

//// changed-a.php
<?hh
class A {
  const type T = arraykey;
}
//// changed-b.php
<?hh
class B extends A {
  // Note that not only the origin changes! of B::T and C::T, so
  // it's a bit weird we type check the use sites of B::T and C::T!
}
//// changed-c.php
<?hh
class C extends B {}
//// changed-use.php
<?hh
function accept_a_const(A::T $_): void {}
function accept_b_const(B::T $_): void {}
function accept_c_const(C::T $_): void {}

function use_a_const(): void { accept_a_const(3); }
function use_b_const(): void { accept_b_const(3); }
function use_c_const(): void { accept_c_const(3); }
