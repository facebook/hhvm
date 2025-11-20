//// foo.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>

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
  public function test1(mixed $m): void {
    $m is C;
    $m as C;
  }
  public function test2(mixed $m): void {
    $m is G<_>;
    $m as G<_>;
  }
  public function test3(mixed $m): void {
    $m is (C, int);
    $m as (C, int);
  }
  public function test4(mixed $m): void {
    $m is T;
    $m as T;
  }
}
