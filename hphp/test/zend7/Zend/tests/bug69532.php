<?php

namespace Foo;

$origins = array();
$profiles = array();
$all_files = array();

array_multisort($origins, SORT_ASC, $profiles, SORT_ASC, $all_files);

?>
===DONE===
