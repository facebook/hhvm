//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

module A {}

//// A.php
<?hh

<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>> class Bing {}

class Quxx { <<__Internal>> public function doFlip(): void {} }

<<__Internal>>
function foobar(): void {}

trait NonInternalCaller {
  public function test(Quxx $q): void {
    // All of these lines are Hack errors: they're `internal` to the module we
    // are currently in, but this trait might be included in another module
    // that doesn't have access to these symbols.
    foobar();
    $q->doFlip();
    $_ = new Bing();
  }
}


<<__Internal>>
trait InternalCaller {
  public function test(Quxx $q): void {
    // This trait is internal, so all of these symbols are OK to access.
    foobar();
    $q->doFlip();
    $_ = new Bing();
  }
}

//// also-A.php
<?hh

<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
trait Quuz {
  <<__Internal>> public function lol(): void {}
}

trait Corge {
  use Quuz;
  <<__Internal>> public function lol(): void {}
}
