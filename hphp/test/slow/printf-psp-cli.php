<?hh

function print_it($mode) {
  echo "echo $mode\n";
  file_put_contents("php://stdout", "file_put_contents $mode\n");
  fwrite(STDOUT, "fwrite $mode\n");
  fflush(STDOUT);
}

function main() {
  print_it('nonpsp');
  register_postsend_function('print_it', 'psp');
}


<<__EntryPoint>>
function main_printf_psp_cli() {
main();
}
