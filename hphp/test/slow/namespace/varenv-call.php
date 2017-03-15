<?php

namespace SomeNS;

function runExtract($args) {
  extract($args);
  var_dump($var1);
}
runExtract(['var1' => 0, 'var2' => 1]);
