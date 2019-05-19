<?php
class Test {
  function __construct() {
    ob_start(
      array(
        $this, 'transform'
      )
    );
  }

  function transform($buffer) {
    return 'success';
  }
}
<<__EntryPoint>> function main() {
$t = new Test;
echo "failure";
}
