//// module_A.php
<?hh
new module A {}

//// module_B.php
<?hh
new module B {}

//// A.php
<?hh

module A;

internal trait T {
  require class C;

  const type TC = shape(
    'channel_id' => string,
  );

  public static function foo(self::TC $data): void {}
}

final class C {
  use T;
}

//// B.php
<?hh

module B;

function main(): void {
  C::foo(1);
}
