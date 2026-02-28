<?hh

class klass implements Countable {
  public function count() :mixed{ return 123; }
}

<<__EntryPoint>> function main(): void {
  $k = new klass;
  var_dump(count($k));
}
