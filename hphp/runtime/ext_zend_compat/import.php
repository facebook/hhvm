<?php
/** Imports a zend extension into HHVM **/

if ($argc != 3) {
  die("Usage: $argv[0] <dir_to_extension> <name_of_extension>\n");
}

$source_root = realpath($argv[1]);
$ext_name = $argv[2];
$systemlib_name = 'ext_'.$ext_name.'.php';
$dest_root = __DIR__.'/'.$ext_name;
$test_root = __DIR__.'/../../test';
print "Importing from $source_root in $dest_root\n";

$objects = new RecursiveIteratorIterator(
  new RecursiveDirectoryIterator($source_root),
  RecursiveIteratorIterator::SELF_FIRST
);

$cpp_files = array();
foreach($objects as $name => $object){
  if (!is_file($name)) {
    continue;
  }

  $dest_dir = $dest_root.'/'.substr($object->getPath(),
    strlen($source_root) + 1);

  if (substr($name, -2, 2) == '.h') {
    install_file($name, $dest_dir.'/'.$object->getFilename());
  } elseif (preg_match("/ext_{$ext_name}.php$/", $name)) {
    install_file($name, $dest_dir.'/'.$object->getFilename());
  } elseif (substr($name, -2, 2) == '.c') {
    $sub_path = substr($dest_dir, strlen($dest_root) + 1);
    $dest_file = ($sub_path ? $sub_path.'/' : '').$object->getFilename().'pp';
    install_file($name, $dest_root.'/'.$dest_file);
    $cpp_files[] = $dest_file;
  } elseif (substr($name, -5, 5) == '.phpt') {
    $dest_base_name = $test_root.'/slow/ext_'.$ext_name.'/'.
      substr($object->getFilename(), 0, -1);
    $sections = parse_phpt($name);
    foreach ($sections as $section_name => $section_text) {
      if ($section_name === 'file') {
        install_file_contents($dest_base_name, $section_text);
      } elseif (in_array($section_name, array('expect', 'expectf',
                                             'expectregex', 'skipif'))) {
        install_file_contents($dest_base_name.'.'.$section_name, $section_text);
      }
    }
  }
}

$srcs = '';
asort($cpp_files);
foreach ($cpp_files as $file) {
  $file = str_replace('//', '/', $file);
  $srcs .= "    '$file',\n";
}
$srcs = trim($srcs);
$capital_name = strtoupper($ext_name);
install_file_contents($dest_root.'/TARGETS', <<<END
# -*- mode: python -*-

cpp_library(
  name='$ext_name',
  srcs=[
    $srcs
  ],
  deps=[
    '@/hphp/runtime:hphp_runtime',
    '@/hphp/runtime/ext_zend_compat:ext_zend_compat_lib',
  ],
  preprocessor_flags=[
    "-Ihphp/runtime/ext_zend_compat/php-src/main/",
    "-Ihphp/runtime/ext_zend_compat/php-src/Zend/",
    "-Ihphp/runtime/ext_zend_compat/php-src/TSRM/",
    "-Ihphp/runtime/ext_zend_compat/php-src/",
  ],
)
END
);

// Write link it into the existing TARGETS
// TODO sort these
install_file_contents(
  __DIR__.'/TARGETS',
  str_replace(
    // Try to be idempotent
    "    '@/hphp/runtime/ext_zend_compat/{$ext_name}',\n".
    "    '@/hphp/runtime/ext_zend_compat/{$ext_name}',",
    "    '@/hphp/runtime/ext_zend_compat/{$ext_name}',",
    str_replace(
      "    '@/hphp/runtime/ext_zend_compat/calendar',",
      "    '@/hphp/runtime/ext_zend_compat/{$ext_name}',\n".
      "    '@/hphp/runtime/ext_zend_compat/calendar',",
      file_get_contents(__DIR__.'/TARGETS')
    )
  )
);

function install_dir($dir) {
  if (!is_dir($dir)) {
    if (!mkdir($dir, 0777, true)) {
      print "Unable to create directory \"$dir\"\n";
      exit(1);
    }
  }
}

function install_file($source, $dest) {
  install_dir(dirname($dest));
  if (!copy($source, $dest)) {
    print "Unable to create file \"$dest\"\n";
    exit(1);
  }
}

function install_file_contents($dest, $text) {
  install_dir(dirname($dest));
  if (file_put_contents($dest, $text) === false) {
    print "Unable to create file \"$dest\"\n";
    exit(1);
  }
}

function parse_phpt($fileName) {
  $contents = file_get_contents($fileName);
  if ($contents === false) {
    print "Unable to read file \"$fileName\"\n";
  }
  $bits = preg_split('/^--([_A-Z]+)--\s*\n/m', $contents, -1,
                     PREG_SPLIT_DELIM_CAPTURE);
  $sections = array();
  for ( $i = 1; $i < count($bits) - 1; $i += 2 ) {
    $sections[strtolower($bits[$i])] = $bits[$i+1];
  }
  return $sections;
}
