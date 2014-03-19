<?php
function tuple($x) {
  echo "Inside tuple\n";
}
class C {
  public function tuple($x) {
    echo "Inside C::tuple\n";
  }
}
class D {
  public static function tuple($x) {
    echo "Inside D::tuple\n";
  }
}
tuple(5);
(new C)->tuple(6);
D::tuple(7);

