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
    "STMT 3.5";
    "STMT 4";
    "STMT 5";
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
    /*DELETE STMTS*/ /*end single line delete*/
    /*DELETE STMTS*/



    /* end delete containing large stmt */
    "STMT 3.5";
    /* DELETE STMTS*/
    /* end multi-stmt delete */
    $var->afun("arg1",
               /*DELETE EXPRS*/ /* end expr delete */
              );
  }
}
