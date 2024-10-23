//// a.php
<?hh

final class A {}

//// ib.php
<?hh

<<__Sealed(B1::class, B2::class)>>
interface IB {}

//// b1.php
<?hh

final class B1 implements IB {}

//// b2.php
<?hh

final class B2 implements IB {}

//// ab.php
<?hh

<<file:__EnableUnstableFeatures('case_types')>>

case type AB = A | IB;

//// test.php
<?hh

<<file:__EnableUnstableFeatures('match_statements')>>

function test(AB $ab): void {
  match ($ab) {
    _: A => hh_expect<A>($ab);
    _: B1 => hh_expect<B1>($ab);
    _: B2 => hh_expect<B2>($ab);
  }
}

////////////////

//// ib.php
<?hh

<<__Sealed(B1::class)>>
interface IB {}

//// b2.php
<?hh

final class B2 {}
