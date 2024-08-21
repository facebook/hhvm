//// a.php
<?hh
// package pkg1
<<__EntryPoint>>
function f(): void {}

//// c.php
<?hh
// package pkg3 (unrelated to pkg1)
<<__EntryPoint, __CrossPackage("pkg1")>>
function test() : void {
  f();
}
