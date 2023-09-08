//// def.php
<?hh

new module foo {}

//// use.php
<?hh

module foo;

public class Foo {
  <<__DynamicallyCallable>>
  internal function foo(): void {}
}
