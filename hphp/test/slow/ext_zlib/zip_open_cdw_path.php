<?php

$archive = "ziparchive_extractto.php.zip";
$full_path = __DIR__.'/'.$archive;

function run($filename) {
  $zip = zip_open($filename);
  var_dump(is_resource($zip));

  $zip = new ZipArchive;
  $res = $zip->open($filename);
  var_dump($res);
}

$initial_cwd = getcwd();

// Relative from CWD path.
chdir(__DIR__);
run($archive);

// Absolute path.
chdir($initial_cwd);
run($full_path);
