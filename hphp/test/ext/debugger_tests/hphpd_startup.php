<?php

function testStartup($x) {
  error_log("testStartup called");
}

if (isset($_ENV['HHVM'])) {
  error_log("HHVM");
}
if (isset($_ENV['HHVM_JIT'])) {
  error_log("HHVM_JIT");
}
