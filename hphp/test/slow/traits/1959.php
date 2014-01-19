<?php

trait Company {
  public function getName() {
    return 'Facebook';
  }
}
class English {
  use Company;
  public function getHi() {
    return "Hi ";
  }
  public function sayHello() {
    echo $this->getHi() . $this->getName();
  }
}
$e = new English();
$e->sayHello();
?>

