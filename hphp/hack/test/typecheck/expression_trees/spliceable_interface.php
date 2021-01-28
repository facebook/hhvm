<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class MySpliceableClass implements Spliceable<Code, Code::TAst, MySpliceableClass> {
  public function visit(Code $v): Code::TAst {
    throw new Exception();
  }
}

function test(): void {
  $x = new MySpliceableClass();
  Code`${$x}`;
}
