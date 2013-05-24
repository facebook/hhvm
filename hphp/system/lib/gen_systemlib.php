<?php

global $scriptPath, $outputPath;
$scriptPath = dirname(__FILE__);
if (!isset($argv[1])) {
  throw new Exception("Usage: gen_systemlib.php <outputPath> [input files]");
}
$outputPath = $argv[1];

function processPhpFile($phpfile, $systemlib_php) {
  $firstchar = true;
  $contents = file_get_contents($phpfile);
  if (preg_match('/\.hhas$/', $phpfile)) {
    $k = 0;
  } else {
    $k = strpos($contents, "\n") + 1;
    $header = trim(substr($contents, 0, $k));
    if ($header !== "<?php") {
      echo "ERROR: Unexpected header in file $phpfile\n";
      throw new Exception("Unexpected header in file $phpfile");
    }
  }
  fwrite($systemlib_php, substr($contents, $k));
}

function populatePhpFiles($input_files) {
  $php_files = array();
  foreach ($input_files as $file) {
    $key = strtolower(basename($file));
    if (!preg_match('/\.(php|hhas)$/', $file)) {
      $errMsg = "ERROR: Encountered non-php file ($file)";
      echo $errMsg . "\n";
      throw new Exception($errMsg);
    }
    if (isset($php_files[$key])) {
      $errMsg = "ERROR: Encountered multiple php files with the same name (" .
                $file . " vs " . $php_files[$key] . ")";
      echo $errMsg . "\n";
      throw new Exception($errMsg);
    }
    // calling Makefile machinery will use the truncated path, full is expected
    $php_files[$key] = $_ENV['FBCODE_DIR'].'/'.$file;
  }
  return $php_files;
}

function genSystemlib($input_files) {
  global $scriptPath;
  global $outputPath;

  $systemlib_php_tempnam = null;
  $systemlib_php = null;

  try {
    $systemlib_php_tempnam = tempnam('/tmp', 'systemlib.php.tmp');
    $systemlib_php = fopen($systemlib_php_tempnam, 'w');
    $phpfiles = populatePhpFiles($input_files);

    fwrite($systemlib_php, "<?hh\n");
    fwrite($systemlib_php, '// @' . 'generated' . "\n\n");
    // There are some dependencies between files, so we output
    // the classes in a certain order so that all classes can be
    // hoisted.
    $initialFiles = array('stdclass.php', 'exception.php', 'arrayaccess.php',
                          'iterator.php', 'splfile.php');
    foreach ($initialFiles as $initialFile) {
      if (isset($phpfiles[$initialFile])) {
        processPhpFile($phpfiles[$initialFile], $systemlib_php);
        unset($phpfiles[$initialFile]);
      }
    }
    foreach ($phpfiles as $key => $phpfile) {
      if (preg_match('/\.php$/', $phpfile)) {
        processPhpFile($phpfile, $systemlib_php);
        unset($phpfiles[$key]);
      }
    }
    fwrite($systemlib_php, "\n");
    if (count($phpfiles)) {
      fwrite($systemlib_php, "\n\n<?hhas\n\n");
      foreach ($phpfiles as $key => $phpfile) {
        processPhpFile($phpfile, $systemlib_php);
        unset($phpfiles[$key]);
      }
    }
    fclose($systemlib_php);
    $systemlib_php = null;
    chmod($systemlib_php_tempnam, 0644);
    `mkdir -p $outputPath`;
    `mv -f $systemlib_php_tempnam $outputPath/systemlib.php`;
  } catch (Exception $e) {
    if ($systemlib_php) fclose($systemlib_php);
    if ($systemlib_php_tempnam) `rm -rf $systemlib_php_tempnam`;
  }
}

genSystemlib(array_slice($argv,2));


