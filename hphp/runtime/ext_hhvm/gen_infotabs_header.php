#!/bin/env php
<?php

# Ignore the --foo arguments passed by fbconfig.
while ($argv[1][0] == '-') {
  array_shift($argv);
}

$output_file = $argv[1];
$outfile_tempnam = tempnam('/tmp', 'ext_hhvm.h.tmp');
$outfile = fopen($outfile_tempnam, 'w');

foreach(array_slice($argv, 2) as $cpp) {
  $header = substr($cpp, 0, -4) . ".h";
  fwrite($outfile, "#include \"$header\"\n");
}
fclose($outfile);
$outfile = null;

if (getenv("INSTALL_DIR")) {
  $output_file = getenv("INSTALL_DIR") . '/' . $output_file;
}
`mv -f $outfile_tempnam $output_file`;
