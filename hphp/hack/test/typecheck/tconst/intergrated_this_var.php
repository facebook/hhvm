<?hh // strict

interface I {}

class C {

  public function get(): this {
    if (true) {
      // Force integration of the $this local variable
    }
    hh_show($this);
    return $this;
  }

  public function get2(): this {
    invariant($this instanceof I, '');
    hh_show($this);
    return $this;
  }
}
