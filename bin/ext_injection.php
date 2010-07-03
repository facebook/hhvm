<?php

chdir(preg_replace('#/bin/ext_injection.php$#', '/src', realpath(__FILE__)));

// parse all these files
$inputs = 'find . -name ext_*.cpp';
$files = array();
exec($inputs, $files);

foreach ($files as $file) {
  $contents = file_get_contents($file);
  if ($contents === false) {
    exit("unable to read $file\n");
  }

  $pattern = '/c_(\w+)::t_(\w+)([^\{\}]*)\{([\n\t ]+)'.
    '(?:\w+INJECTION\([\w:, ]+\);[\n\t ]+)?([^\}])/s';
  $replace = "c_\${1}::t_\${2}\${3}{\n  ".
    "INSTANCE_METHOD_INJECTION_BUILTIN(\${1}, \${1}::\${2});\${4}\${5}";

  $replaced = preg_replace($pattern, $replace, $contents);

  $pattern = '/c_(\w+)::ti_(\w+)([^\{\}]*)\{([\n\t ]+)'.
    '(?:\w+INJECTION\([\w:, ]+\);[\n\t ]+)?([^\}])/s';
  $replace = "c_\${1}::ti_\${2}\${3}{\n  ".
    "STATIC_METHOD_INJECTION_BUILTIN(\${1}, \${1}::\${2});\${4}\${5}";

  $replaced = preg_replace($pattern, $replace, $replaced);

  if ($replaced && $replaced !== $contents) {
    file_put_contents($file, $replaced);
    print "updated $file\n";
  }
}
