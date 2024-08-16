//// foo.php
<?hh
// package pkg2
class C {}
class G<T> {}
enum E : int {
  X = 1;
}
type T = int;

//// bar.php
<?hh
// package pkg1

class Bar {
  public function bar(mixed $m):void {
    $m as C;
    $m is C;

    $m is G<_>;
    $m as G<_>;

    $m is (C, int);
    $m as (C, int);

    $m as E;
    $m is E;

    $m as T;
    $m is T;
  }
}
