<?php

print "Test begin\n";

function __autoload($cls) {
  echo "__autoload $cls\n";
}

require "autoload5.php";

print "Test end\n";
