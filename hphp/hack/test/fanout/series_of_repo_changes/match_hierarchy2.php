//// a.php
<?hh

final class A {}

//// ib.php
<?hh

<<__Sealed(B1::class, B2::class, IC::class)>>
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

//// ic.php
<?hh

<<__Sealed(C1::class, C2::class)>>
interface IC extends IB {}

//// c1.php
<?hh

final class C1 implements IC {}

//// c2.php
<?hh

final class C2 implements IC {}

//// test.php
<?hh

<<file:__EnableUnstableFeatures('match_statements')>>

function test(AB $ab): void {
  match ($ab) {
    _: A => hh_expect<A>($ab);
    _: B1 => hh_expect<B1>($ab);
    _: B2 => hh_expect<B2>($ab);
    _: C1 => hh_expect<C1>($ab);
    _: C2 => hh_expect<C2>($ab);
  }
}

////////////////

//// c3.php
<?hh

final class C3 extends IC {}

//// ic.php
<?hh

<<__Sealed(C1::class, C2::class, C3::class)>>
interface IC extends IB {}
