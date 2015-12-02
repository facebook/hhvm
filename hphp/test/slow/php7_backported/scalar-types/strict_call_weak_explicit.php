<?php

declare(strict_types=1);

// file that's explicitly weak
require 'strict_call_weak_explicit_2.inc';

// Will succeed: Function was declared in weak mode, but that does not matter
// This file uses strict mode, so the call is strict, and float denied for int
function_declared_in_weak_mode(1.0);
?>
