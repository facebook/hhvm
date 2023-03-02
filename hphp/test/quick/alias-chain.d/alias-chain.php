<?hh

class Foo {
  public int $x;
}

function fiz(Bar $r): Bar {
  $r->x = $r->x + 1;
  return $r;
}
<<__EntryPoint>>
function main_entry(): void {
  $f = new Foo();
  $f->x = 10;
  $z = fiz($f);
  var_dump($z->x);
}
