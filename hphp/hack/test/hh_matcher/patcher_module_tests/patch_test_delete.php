//// matcherpattern.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class __DONOTCARE {
  public function __DONOTCARE(): void {
    "STMT 1";
    "STMT 2";
    if (true) {
      "STMT 3 true";
    } else {
      "STMT 3 false";
    }
    "STMT 4";
    $var->afun("arg1",
               "arg2", "arg3"
              );
  }
}
//// matchertarget.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class __DONOTCARE {
  public function __DONOTCARE(): void {
    "STMT 1";
    /*DELETE STMTS*/




    /*end delete*/
    "STMT 4";
    $var->afun("arg1",
               /*DELETE EXPRS*/ /*end delete*/
              );
  }
}
