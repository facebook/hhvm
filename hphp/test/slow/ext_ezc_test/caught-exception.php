<?php
var_dump(ezc_try_call(function () {
  throw new Exception("test");
}));

?>
