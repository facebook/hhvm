////file1.php
<?hh
<<__SupportDynamicType>>
class Cov<+T> { }
<<__SupportDynamicType>>
class Derived<+T> extends Cov<T> { }
newtype MyMemberOf<+TType> as TType = TType;
newtype MyLabel<TType> = mixed;
newtype N as Cov<~int> = Derived<~int>;
newtype P<+T> as Cov<T> = Cov<T>;
////file2.php
<?hh

function valueOf<TType as supportdyn<mixed> >(~MyLabel<TType> $label): ~MyMemberOf<TType> {
  throw new Exception("A");
}

function getE(~MyLabel<int> $x): ~int {
  return valueOf<_>($x);
}

function testN(N $n): ~Cov<int> {
  return $n;
}

function testP(P<~int> $p) : ~Cov<int> {
  return $p;
}
