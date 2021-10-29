//// file1.php
<?hh
<<file:__EnableUnstableFeatures('readonly')>>

class Foo<T> {


  final public function toNotEqual(
    readonly T $expected,
    mixed ...$args
  ): void {
    PHPUnit_Util_Type::toString($expected);
  }

}

//// file2.php
<?hh
abstract final class  PHPUnit_Util_Type {
 public static function toString(mixed $expected, bool $short = false): string {
   return "";
 }
}
