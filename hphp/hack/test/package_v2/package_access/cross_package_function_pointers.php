//// foo.php
<?hh
// package pkg1 (does not include pkg2)
function main(): void {
  C::foo<>;
  bar<>;
}

//// bar.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
class C {
  public static function foo(): void {}
}
function bar(): void {}
