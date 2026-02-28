//// def.php
<?hh

newtype TDef = int;
class Vals {
  const TDef foo = 0;
}

//// use.php
<?hh

class C {
  private TDef $x = Vals::foo;

  public function f() {
    if ($this->x !== Vals::foo) {
      throw new Exception('s' . $this->x);
    }
  }
}
