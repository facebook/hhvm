//// phpdecl.php
<?php // decl

class C {}

//// partial.php
<?hh

class D extends C {
  public function f() {
    return $this->g();
  }
}
