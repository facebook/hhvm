<?hh

final class :foo:bar {
  attribute string name, string color, string style, int number @required;

  public function genRender() {}
}

function main(): void {
  <foo:bar name={'Serenity'} color={'Gray'} AUTO332/>;
}
