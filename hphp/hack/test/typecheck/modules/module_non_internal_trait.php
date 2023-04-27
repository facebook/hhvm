//// modules.php
<?hh


new module A {}

//// A.php
<?hh


module A;

internal class Bing {}

class Quxx { internal function doFlip(): void {} }

internal function foobar(): void {}

trait NonInternalCaller {
  public function test(Quxx $q): void {
    // All of these lines are Hack errors: they're `internal` to the new module we
    // are currently in, but this trait might be included in another module
    // that doesn't have access to these symbols.
    foobar();
    $q->doFlip();
    $_ = new Bing();
  }
}

internal trait InternalCaller {
  public function test(Quxx $q): void {
    // This trait is internal, so all of these symbols are OK to access.
    foobar();
    $q->doFlip();
    $_ = new Bing();
  }
}

//// also-A.php
<?hh


module A;

internal trait Quuz {
  internal function lol(): void {}
}

trait Corge {
  use Quuz;
  internal function lol(): void {}
}
