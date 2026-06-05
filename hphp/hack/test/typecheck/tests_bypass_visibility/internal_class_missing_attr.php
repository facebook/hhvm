//// modules.php
<?hh

new module bar {}

//// bar.php
<?hh

module bar;

internal class BadClass {
  <<__TestsBypassVisibility>>
  internal function secret(): void {} // error: class is internal but lacks attribute

  <<__TestsBypassVisibility>>
  internal static function stat(): void {} // error: class is internal but lacks attribute

  internal function ok(): void {} // ok: no attribute on method
}
