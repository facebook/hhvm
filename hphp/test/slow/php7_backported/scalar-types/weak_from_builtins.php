<?php

declare(strict_types=1);

function print_str(string $foo) { var_dump($foo); }

$input = [1, 2, 3];
array_map('print_str', $input);
