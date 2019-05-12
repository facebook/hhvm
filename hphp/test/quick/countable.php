<?hh

class klass implements Countable {
  public function count() { return 123; }
}

<<__EntryPoint>> function main(): void {
  $k = new klass;
  var_dump(count($k));
}
