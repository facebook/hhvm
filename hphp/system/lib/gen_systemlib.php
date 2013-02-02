<?php

global $scriptPath, $outputPath;
$scriptPath = dirname(__FILE__);
if (!isset($argv[1])) {
  throw new Exception("Usage: gen_systemlib.php <outputPath>");
}
$outputPath = $argv[1];

function processPhpFile($phpfile, $systemlib_php) {
  $firstchar = true;
  $contents = file_get_contents($phpfile);
  $i = 0;
  $k = strpos($contents, "\n") + 1;
  $header = trim(substr($contents, 0, $k));
  if ($header !== "<?php") {
    echo "ERROR: Unexpected header in file $phpfile\n";
    throw new Exception("Unexpected header in file $phpfile");
  }
  fwrite($systemlib_php, substr($contents, $k));
}

function searchDirForPhpFiles($searchPath, &$phpfiles) {
  $list = scandir($searchPath);
  $files = array();
  foreach ($list as $name) {
    if ($name === '.' || $name === '..') continue;
    $files[] = $searchPath . '/' . $name;
  }
  for ($i = 0; $i < count($files); ++$i) {
    $file = $files[$i];
    if (is_dir($file)) {
      $list = scandir($file);
      foreach ($list as $name) {
        if ($name === '.' || $name === '..') continue;
        $files[] = $file . '/' . $name;
      }
      continue;
    }
    if (!preg_match('/\.php$/', $file)) continue;
    $key = strtolower(basename($file));
    if (isset($phpfiles[$key])) {
      $errMsg = "ERROR: Encountered multiple php files with the same name (" .
                $file . " vs " . $phpfiles[$key] . ")";
      echo $errMsg . "\n";
      throw new Exception($errMsg);
    }
    $phpfiles[$key] = $file;
  }
}

function genSystemlib() {
  global $scriptPath;
  global $outputPath;

  $systemlib_php_tempnam = null;
  $systemlib_php = null;

  try {
    $systemlib_php_tempnam = tempnam('/tmp', 'systemlib.php.tmp');
    $systemlib_php = fopen($systemlib_php_tempnam, 'w');
    // Build up a list of all the .php files in hphp/system/classes
    $phpfiles = array();
    $searchPath = realpath($scriptPath . '/../../system/classes');
    searchDirForPhpFiles($searchPath, $phpfiles);
    $searchPath = realpath($scriptPath . '/../../system/classes_hhvm');
    searchDirForPhpFiles($searchPath, $phpfiles);

    fwrite($systemlib_php, "<?php\n");
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
    foreach ($phpfiles as $phpfile) {
      processPhpFile($phpfile, $systemlib_php);
    }
    fwrite($systemlib_php, "\n");

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

genSystemlib();


