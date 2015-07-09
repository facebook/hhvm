<?hh // strict

// This file test is a basic test that new class_id produces a expression
// dependent type.

<<__ConsistentConstruct>>
abstract class C2 {
  abstract const type TFoo;

  protected function __construct() {}

  protected function setFoo(this::TFoo $foo): void {}

  public static function create(this::TFoo $foo): this {
    // new static() will produce the expression dependent type
    // `static.
    $c = new static();
    hh_show($c);

    // When we invoke setFoo(), this::TFoo will be resolved to static::TFoo, so
    // no error occurs.
    $c->setFoo($foo);

    return $c;
  }
}
