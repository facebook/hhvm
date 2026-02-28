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

//// ab.php
<?hh

<<file:__EnableUnstableFeatures('case_types')>>

case type ABC = A | B1 | B2 | C1 | C2;

//// test.php
<?hh

<<file:__EnableUnstableFeatures('match_statements')>>

function test(ABC $ab): void {
  match ($ab) {
    _: A => hh_expect<A>($ab);
    _: IB => hh_expect<IB>($ab);
  }
}

////////////////

//// ic.php
<?hh

<<__Sealed(C1::class)>>
interface IC extends IB {}

//// C2.php
<?hh

final class C2 {}
