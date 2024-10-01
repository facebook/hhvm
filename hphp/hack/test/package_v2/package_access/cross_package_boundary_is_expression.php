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
  public function bar(mixed $m):void {
    $m is C;
    $m is G<_>;
    $m is (C, int);
    $m is E;
    $m is T;
  }
}
