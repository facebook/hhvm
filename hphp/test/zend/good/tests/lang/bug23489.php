<?php
class Test {
  function Test() {
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
