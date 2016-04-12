<?php

declare(strict_types=1);

// file that's implicitly weak
require 'strict_include_weak_2.inc';

// calls within that file should stay weak, despite being included by strict file
?>
