<?php

isset($argv[1]) || die("missing {name}:\n\tphp new_cmd.php {name}\n");
$NAME = strtoupper($Name = ucfirst($name = strtolower($argv[1])));

foreach (array('h', 'cpp') as $ext) {
  exec("cp -f cmd.$ext.template cmd_$name.$ext");
  foreach (array('machine' => $name, 'Machine' => $Name, 'MACHINE' => $NAME)
           as $from => $to) {
    exec("perl -p -i -n -e 's/$from/$to/g' cmd_$name.$ext");
  }
}

if (!isset($argv[2])) {
  $include = "#include <runtime/eval/debugger/cmd/cmd_$name.h>";
  exec("perl -p -i -n -e 's!$include\n!!sg' all.h");
  exec("perl -p -i -n -e 's!^(//tag: new_cmd\.php.*)$!$include\n$1!' all.h");
}

