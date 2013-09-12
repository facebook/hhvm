<?php
/** Imports a zend extension into HHVM **/

if ($argc != 3) {
  die("Usage: $argv[0] <dir_to_extension> <name_of_extension>\n");
}

$source_root = realpath($argv[1]);
$ext_name = $argv[2];
$dest_root = __DIR__.'/'.$ext_name;
print "Importing from $source_root in $dest_root\n";

mkdir($dest_root);
$objects = new RecursiveIteratorIterator(
  new RecursiveDirectoryIterator($source_root),
  RecursiveIteratorIterator::SELF_FIRST
);

$cpp_files = array();
foreach($objects as $name => $object){
  if (!is_file($name)) {
    continue;
  }

  if (substr($name, -2, 2) == '.h') {
    $dest_dir = $dest_root.'/'.$object->getSubpath();
    mkdir($dest_dir);
    copy($name, $dest_dir.'/'.$object->getFilename());

  } elseif (substr($name, -2, 2) == '.c') {
    $sub_path = $object->getSubpath();
    mkdir($dest_root.'/'.$sub_path);
    $dest_file = ($sub_path ? $sub_path.'/' : '').$object->getFilename().'pp';
    copy($name, $dest_root.'/'.$dest_file);
    $cpp_files[] = $dest_file;
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
file_put_contents($dest_root.'/TARGETS', <<<END
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
    "-DCOMPILE_DL_$capital_name",
  ],
)
END
);

// Write link it into the existing TARGETS
// TODO sort these
file_put_contents(
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

print "Generating $ext_name.idl.json file from php.net\n";
$idl_dir = __DIR__.'/../../system/idl';
exec("php $idl_dir/newext.php $ext_name");
rename("$ext_name.idl.json", "$idl_dir/$ext_name.idl.json");
