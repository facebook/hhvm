<?php

$command = "exit 2";
passthru($command, $return);
print "$return\n";
