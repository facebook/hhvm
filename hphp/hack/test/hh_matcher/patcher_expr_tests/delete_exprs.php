//// tosearch.php
<?hh // strict

class Foo {
  public function bar(): void {
    $val = true;
    "Stmt 1";
    if ($val) {
      "nested stmt";
      $var->afun("arg1", "arg2", "arg3");
    } else {
      "else clause";
      $var->afun("arg1", "arg2", "arg3");
    }
    "Not nested";
    $var->afun("arg1", "arg2", "arg3");
  }
}

//// matcherpattern.php
<?hh

"arg2";

//// matchertarget.php
<?hh

/*DELETE EXPRS*/ /*end delete*/
