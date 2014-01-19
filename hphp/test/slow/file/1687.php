<?php

$tmp_dir='/tmp/tcr_tmp_dir'.getmypid();
mkdir($tmp_dir);
print_r(glob($tmp_dir.'/*'));
rmdir($tmp_dir);
