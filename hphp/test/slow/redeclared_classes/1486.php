<?hh

if (isset($g)) {
  include '1486-1.inc';
}
else {
  include '1486-2.inc';
}
class d extends c {
  private $b = 'b';
  function t2() {
    foreach ($this as $k => $v) {
      var_dump($v);
    }
  }
}
$x = new d;
$x->t2();
