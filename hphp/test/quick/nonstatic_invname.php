<?hh

class Blah {
  public function __call(string $a, $b) {
    var_dump(substr($a, 0, 4));
    $a[2] = 'a';
    var_dump(substr($a, 0, 4));
  }
}

function heh() {
  return "zzzz" . mt_rand();
}

function main(Blah $l) {
  $l->{heh()}();
}
<<__EntryPoint>> function main_entry(): void {
main(new Blah);
}
