<?php
class C {
  public function __construct() {
    define("Foo", 10);
  }
}
echo (define("GREETING", "Hello"));
echo (GREETING) ;
new C();
echo (Foo) ;
