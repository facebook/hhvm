//// one.php
<?hh

class One {
  public function foo() {
    return 1;
  }
}

//// two.php
<?hh

class Two extends One {
  public function foo() {
    return 2;
  }
}

//// three.php
<?hh

class Three extends Two {
  public function foo() {
    return 3;
  }
}

