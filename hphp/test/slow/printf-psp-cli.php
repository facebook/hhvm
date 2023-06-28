<?hh

function print_it($mode) :mixed{
  echo "echo $mode\n";
  file_put_contents("php://stdout", "file_put_contents $mode\n");
  fwrite(HH\stdout(), "fwrite $mode\n");
  fflush(HH\stdout());
}

<<__EntryPoint>>
function main_printf_psp_cli() :mixed{
  print_it('nonpsp');
  register_postsend_function(() ==> { print_it('psp'); });
}
