<?hh

class d extends c {
  private $b = 'b';
  function t2() {
    foreach ($this as $k => $v) {
      var_dump($v);
    }
  }
}
