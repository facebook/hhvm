<?hh

trait TMyTrait {
  public function doNothing(): void {}
}

trait TMyTrait2 {
  use TMyTrait;

  public function doMoreNothing(): void {}
}

trait TMyTrait3 {
  public function doYetMoreNothing(): void {}
}

class MyTraitUsingClass {
  use TMyTrait2;
  use TMyTrait3;
}
