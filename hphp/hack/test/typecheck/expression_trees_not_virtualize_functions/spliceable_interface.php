<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class MySpliceableClass implements Spliceable<ExampleDsl, ExampleDsl::TAst, MySpliceableClass> {
  public function visit(Code $v): ExampleDsl::TAst {
    throw new Exception();
  }
}

function test(): void {
  $x = new MySpliceableClass();
  ExampleDsl`${$x}`;
}
