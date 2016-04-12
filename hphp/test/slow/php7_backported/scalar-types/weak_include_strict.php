<?php

// implicitly weak code

// file with strict_types=1
require 'weak_include_strict_2.inc';

// calls within that file should stay strict, despite being included by weak file
?>
