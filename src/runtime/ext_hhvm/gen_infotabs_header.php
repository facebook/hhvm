<?php

$output_file = $argv[1];
$outfile_tempnam = tempnam('/tmp', 'ext_hhvm.h.tmp');
$outfile = fopen($outfile_tempnam, 'w');

foreach(array_slice($argv, 2) as $cpp) {
  $header = substr($cpp, 0, -4) . ".h";
  fwrite($outfile, "#include \"$header\"\n");
}

fclose($outfile);
$outfile = null;
`mv -f $outfile_tempnam $output_file`;
