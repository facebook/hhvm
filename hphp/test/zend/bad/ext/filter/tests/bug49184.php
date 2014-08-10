<?php
$_ENV[HTTP_X_FORWARDED_FOR] = example.com;
_filter_snapshot_globals();

  var_dump(filter_input(INPUT_SERVER, "HTTP_X_FORWARDED_FOR", FILTER_UNSAFE_RAW));
  var_dump($_SERVER["HTTP_X_FORWARDED_FOR"]);
  var_dump(getenv("HTTP_X_FORWARDED_FOR"));
  var_dump("done");
?>
