<?hh

class K {
  public function meth(mixed $_): void {}
}

<<__EntryPoint>>
function foo(): void {
  $k = new K();
  $k->meth(dict[]);
}
