<?php
/**
 * Test for PHP bug #50394 (PHP 5.3.x conversion to null only, not 5.2.x)
 */
class PhpRefCallBugTester {
  public $ok = false;
  function __call( $name, $args ) {
    $old = error_reporting( E_ALL & ~E_WARNING );
    call_user_func_array( array( $this, 'checkForBrokenRef' ), $args );
    error_reporting( $old );
  }
  function checkForBrokenRef( &$var ) {
    if ( $var ) {
      $this->ok = true;
    }
  }
  function execute() {
    $var = true;
    call_user_func_array( array( $this, 'foo' ), array( &$var ) );
  }
}
$test = new PhpRefCallBugTester;
$test->execute();
if (!$test->ok) {
  echo "Test failed\n";
} else {
  echo "Test passed\n";
}

