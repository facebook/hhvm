<?hh // partial

class A {
  <<__Rx, __Mutable>>
  public function f(): void {
    $b = varray[$this];
    $b1 = varray[$this];
    $c = Vector {$this};
    $d = Map {"x" => $this};
    $e = dict["x" => $this];
    $e1 = darray["x" => $this];
    $f = Pair {$this, $this};
    $g = shape('x' => $this);

    $l = () ==> { $this; };
    $i = $this;
  }

  <<__Rx, __MaybeMutable>>
  public function g(): void {
    $b = varray[$this];
    $b1 = varray[$this];
    $c = Vector {$this};
    $d = Map {"x" => $this};
    $e = dict["x" => $this];
    $e1 = darray["x" => $this];
    $f = Pair {$this, $this};
    $g = shape('x' => $this);

    $l = () ==> { $this; };
    $i = $this;
  }
}
