//// text.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

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
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  public function bar(): void {
    "__SKIPANY";
    {
      "__KSTAR";
      $var->afun("arg1",
                 "arg2",
                 "arg3");
    }
  }
}

//// matchertarget.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  public function bar(): void {
    "__SKIPANY";
    {
      "__KSTAR";
      $var->afun("arg1",
                 /*DELETE EXPRS*/ /*end delete*/
                 "arg3");
    }
  }
}
