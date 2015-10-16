//// tosearch.php
<?hh // strict

/** IMPORTANT: Expected output for this test is wrong. There should not be
 * a fatal exception raised. Once this bug in Matcher.patch_expr is fixed,
 * the expected output should be updated, and this comment should be removed
 */

class Foo {
  public function bar(): void {
    $foo = array(1, 2, 3, 4, 5);
    return;
  }
}

//// matcherpattern.php
<?hh

$foo =
  array("__KSTAR_META_A");

//// targetpattern.php
<?hh

$foo =
  /*REPLACE EXPR*/Vector {"__KSTAR_META_A"};/**/
