<?hh

class c {
  public $prop = 'old value';
}

function propget($o, $p) {
  return $o->$p;
}

function propset($o, $p, $v) {
  $o->$p = $v;
}

<<__EntryPoint>> function main(): void {
  $c = new c();
  var_dump(propget($c, 'prop'));
  propset($c, 'prop', 'new value');
  var_dump(propget($c, 'prop'));

  var_dump(propget($c, 'fakeprop'));
  propset($c, 'fakeprop2', 'blah');
}
