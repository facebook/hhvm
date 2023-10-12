<?hh // strict

type TDef = int;

class C {
  private TDef $x = 0;

  public function f() {
    if ($this->x !== 0) {
      throw new Exception('s' . $this->x);
    }

    $this->blah();
  }

}
