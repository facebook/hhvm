//// def.php
<?hh

new module foo {}
//// use.php
<?hh

module foo;

internal class Foo {
  public function __construct(<<__SoftInternal>> internal int $x,
    <<__SoftInternal>> int $w, // not ok
  ) {}
  public function foo(<<__SoftInternal>> int $l) // not ok
  : void {}
}
