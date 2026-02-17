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

internal class InternalForSealed {}

<<__Sealed(InternalForSealed::class)>>
interface SealedInterface {}

<<__Sealed(InternalForSealed::class, Foo::class)>>
abstract class SealedClass {}

class ClassWithSealedMethod {
  <<__Sealed(InternalForSealed::class)>>
  public function sealedMethod(): void {}
}

function uses_internal_outside_sealed(): void {
  $x = InternalForSealed::class;
}
