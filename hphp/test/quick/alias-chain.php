<?hh

HH\autoload_set_paths(
  array(
    'type' => array('bar' => 'alias-chain-1.inc', 'baz' => 'alias-chain-2.inc'),
  ),
  __DIR__.'/'
);

class Foo {
  public int $x;
}

function fiz(Bar $r): Bar {
  $r->x = $r->x + 1;
  return $r;
}

$f = new Foo();
$f->x = 10;
$z = fiz($f);
var_dump($z->x);
