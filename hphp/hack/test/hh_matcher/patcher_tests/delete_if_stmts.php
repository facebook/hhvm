//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    $var = 1;
    if (true) {
      "Stmt1 true";
    } else {
      "Stmt1 false";
    }
    "STMT2";
    if (true) {
      if (true) {
        "nested true true";
      } else {
        "nested true false";
      }
    } else {
      "nonnested false";
    }
    return;
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    "__SKIPANY";
    {
      "__KSTAR";
      if (true) {
        "__KSTAR";
      } else {
        "__KSTAR";
      }
    }
    return;
  }
}

//// targetpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    "__SKIPANY";
    {
      "__KSTAR";
      /*DELETE STMTS*/



      /*end delete*/
    }
    return;
  }
}
