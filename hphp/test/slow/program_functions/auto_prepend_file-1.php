<?hh 
<<__EntryPoint>>
function main_auto_prepend_file_1() {
  prepend();
  echo "main\n";
  var_dump(function_exists('append'));
}
