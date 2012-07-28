<?php
function foo() {
  $sc = new SoapClient(null,
                       array('location' =>
                             "http://fizzle/does-not-exist.php",
                             'uri' => "http://does-not-exit-uri/"));
  try {
    var_dump($sc->NotASoapFunction());
  } catch (Exception $e) {
    echo "Caught exception\n";
  }
}
foo();
