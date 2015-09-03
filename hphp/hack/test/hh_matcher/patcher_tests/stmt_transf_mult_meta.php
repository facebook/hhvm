//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    if (true) {
      "stmt 1";
      "stmt 2";
    }
    return;
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    if (true) {
      "__KSTAR_META_A";
    }
    return;
  }
}

//// targetpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    /*REPLACE STMT*/while (true) {
      "__KSTAR_META_A";
    }/**/
    return;
  }
}
