<?hh

interface I {
  public function getFoo<<<__Enforceable>>reify Tfoo>(): ?Tfoo;

  public function getBar<<<__Enforceable>>reify Tbar as IBaaaaaaaaaaaaaaaaaaar>(): ?Tbar;
}
