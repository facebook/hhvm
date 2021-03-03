<?hh

final class :foo:bar {
  attribute string name, string color;

  public function genRender() {}
}

function main(): void {
  $x = <foo:bar name=""/>;
  $x->AUTO332
}
