//// file1.php
<?hh
function test<T as C>(
  T $c,
  A $a,
): void {
  $b = $c->b();
  $_ = new $b($a);
}
//// file2.php
<?hh
interface A {}
final class B<Ta as A> {
  public function __construct(Ta $a) {}
}
interface C {
  abstract const type Ta as A;
  public function b(): classname<B<this::Ta>>;
}
