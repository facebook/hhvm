<?hh // strict

namespace NS_require;

//require 'unknown.php';
require 'limits.php';
require('mycolors.php');
require('return_none.php');
require('return_with_value.php');

function main(): void {
///*
// Try to access constants defined in a required file

  if (defined("MY_MIN"))
    echo "MY_MIN is defined with value >" . constant("MY_MIN") . "\n";
  else
    echo "MY_MIN is not defined\n";
//*/

// get the set of included files

  print_r(get_included_files());
}

/* HH_FIXME[1002] call to main in strict*/
main();
