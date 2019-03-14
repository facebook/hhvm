//// file1.php
<?hh // partial

newtype ID<+T> = int;
newtype UID<+T> as ID<T> = int;
newtype OID<+T> as ID<T> = int;

//// file2.php
<?hh // partial

class A {}
class B extends A {}

function takesIDA(ID<A> $x): void {}
function test(UID<B> $x): void {
  takesIDA($x);
}
