<?php

try {
  ezc_call(function () {
    throw new Exception("test");
  });
} catch (Exception $e) {
  var_dump($e);
}

?>
