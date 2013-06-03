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
echo "Done\n";

