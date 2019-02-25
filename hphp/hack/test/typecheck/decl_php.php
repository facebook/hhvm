//// phpdecl.php
<?php // decl

class C {}

//// partial.php
<?hh // partial

class D extends C {
  public function f() {
    return $this->g();
  }
}
