<?php

$fp = fopen(__DIR__.'/../../sample_dir/file', 'r');
var_dump(pclose($fp));
