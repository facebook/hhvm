//// one.php
<?hh // partial

class One {
  public function foo() {
    return 1;
  }
}

//// two.php
<?hh // partial

class Two extends One {
  public function foo() {
    return 2;
  }
}

//// three.php
<?hh // partial

class Three extends Two {
  public function foo() {
    return 3;
  }
}

