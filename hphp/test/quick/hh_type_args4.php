<?hh
class A<T> {
  public function getT(T $x): T {
    return $x;
  }
}
class B<T> extends A<T> { }
function foo<T, Q as A<T> >(Q $thing, T $item): T {
  return $thing->getT($item);
}
class C<T as A<int>> {}
class D<T as :x:base> {}
class E<T as A<:x:base>> {}
echo "Done\n";

