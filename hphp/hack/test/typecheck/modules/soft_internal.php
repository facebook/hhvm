////file1.php
<?hh

new module foo {}

////file2.php
<?hh


module foo;
<<__SoftInternal>>
internal function foo(): void {}

<<__SoftInternal>>
function bar(): void {}

<<__SoftInternal>>
class Bar {
  <<__SoftInternal>>
  public function bar(): void {}

  <<__SoftInternal>>
  public static function bar2(): void {}
  <<__SoftInternal>>
  public int $x = 5;
}


