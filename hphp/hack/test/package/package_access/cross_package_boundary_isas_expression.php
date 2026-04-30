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
    $m ?as C;
    $m as C;
  }
  public function test2(mixed $m): void {
    $m is G<_>;
    $m ?as G<_>;
    $m as G<_>;
  }
  public function test3(mixed $m): void {
    $m is (C, int);
    $m ?as (C, int);
    $m as (C, int);
  }
  public function test4(mixed $m): void {
    $m is T;
    $m ?as T;
    $m as T;
  }
  public function test5(mixed $m): void {
    $m is shape('k' => C);
    $m ?as shape('k' => C);
    $m as shape('k' => C);
  }
  public function test6(mixed $m): void {
    $m as shape('k' => shape('inner' => C));
  }
  public function test7(mixed $m): void {
    $m as (shape('k' => C), int);
  }
  public function test8(mixed $m): void {
    $m as (int, C);
  }
  public function test9(mixed $m): void {
    $m as (int, int, C);
  }
}
