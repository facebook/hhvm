<?hh // strict

class TestCloneWhatBase {
  public function clone(): this {
    return $this;
  }
}
final class TestCloneWhat extends TestCloneWhatBase {
  <<__Override>>
  public function clone(): this {
    return $this;

  }
}

echo "Hello\n";
