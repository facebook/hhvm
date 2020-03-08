<?hh // strict

namespace NS_error_control;

function main(): void {
  $infile = fopen("NoSuchFile.txt", 'r');
  $infile = @fopen("NoSuchFile.txt", 'r');
}

/* HH_FIXME[1002] call to main in strict*/
main();
