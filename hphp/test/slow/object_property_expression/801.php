<?hh

class X {
  function notref($_) {

  }
  function bar() {
    $this->notref($this->priv);
  }
}
class Y extends X {
 private $priv;
 }
class Z extends Y {
}

<<__EntryPoint>>
function main_801() {
;
$z = new Z;
$z->bar();
var_dump($z);
$y = new Y;
$y->bar();
var_dump($y);
}
