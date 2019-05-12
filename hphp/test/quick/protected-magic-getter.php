<?hh

class Whatever {
  protected $blah;

  public function __get($name) { var_dump($name); }
}

<<__EntryPoint>> function main(): void {
  $l = new Whatever();
  $l->blah += 2;
  var_dump($l);
}
