//// def.php
<?hh

new module foo {}
//// use.php
<?hh

module foo;

internal class Foo {
  public function foo(): void {
  $y = Foo::class;
  $y = static::class;
  }
}

public class Bar extends Foo {
  public function test(): void {
    $y = parent::class;
  }

}
