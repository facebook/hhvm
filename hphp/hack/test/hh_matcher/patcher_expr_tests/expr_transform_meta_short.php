//// tosearch.php
<?hh // strict

/** IMPORTANT: Expected output for this test is wrong. There should not be
 * a fatal exception raised. Once this bug in Matcher.patch_expr is fixed,
 * the expected output should be updated
 */

class Foo {
  public function bar(): void {
    $foo = array(1);
    $bar = array(5);
    return;
  }
}

//// matcherpattern.php
<?hh

array("__ANY_META_A");

//// targetpattern.php
<?hh

/*REPLACE EXPR*/Vector {"__META_A"};/**/
