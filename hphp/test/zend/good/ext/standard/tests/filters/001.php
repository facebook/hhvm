<?php

var_dump(stream_filter_register("", ""));
var_dump(stream_filter_register("test", ""));
var_dump(stream_filter_register("", "test"));
var_dump(stream_filter_register("------", "nonexistentclass"));
try { var_dump(stream_filter_register(array(), "aa")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(stream_filter_register("", array())); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
?>
