<?php

chdir(preg_replace('#/hphp/tools/ext_injection.php$#', '/hphp', realpath(__FILE__)));

// parse all these files
$inputs = 'find . -name ext_*.cpp | '.
  'grep -Ev \'/ext/[^/]+.ext_hhvm.cpp\' |'.
  'grep -v facebook';
$files = array();
exec($inputs, $files);

foreach ($files as $file) {
  // Skip injection macros for the collection and closure classes
  $basename = basename($file);
  if (($basename == 'ext_collection.cpp') ||
      ($basename == 'ext_closure.cpp')) {
    continue;
  }

  $contents = file_get_contents($file);
  if ($contents === false) {
    exit("unable to read $file\n");
  }

  $pattern = '/c_(\w+)::t_(\w+)([^\{\}]*)\{([\n\t ]+)'.
    '(?:\w+INJECTION(?:_BUILTIN|)\([\w:, ]+\);[\n\t ]+)?([^\}])/s';
  $replace = "c_\${1}::t_\${2}\${3}{\n  ".
    "INSTANCE_METHOD_INJECTION_BUILTIN(\${1}, \${1}::\${2});\${4}\${5}";

  $replaced = preg_replace($pattern, $replace, $contents);

  $pattern = '/c_(\w+)::ti_(\w+)([^\{\}]*)\{([\n\t ]+)'.
    '(?:\w+INJECTION(?:_BUILTIN|)\([\w:, ]+\);[\n\t ]+)?([^\}])/s';
  $replace = "c_\${1}::ti_\${2}\${3}{\n  ".
    "STATIC_METHOD_INJECTION_BUILTIN(\${1}, \${1}::\${2});\${4}\${5}";

  $replaced = preg_replace($pattern, $replace, $replaced);

  if ($replaced && $replaced !== $contents) {
    file_put_contents($file, $replaced);
    print "updated $file\n";
  }
}
