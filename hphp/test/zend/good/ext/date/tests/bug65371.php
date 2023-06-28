<?hh

function p($str)
:mixed{
  echo $str, "\n";
  echo strftime($str), "\n";
  echo bin2hex($str), "\n";
  echo bin2hex(strftime($str));
}
<<__EntryPoint>>
function main_entry(): void {

  setlocale(LC_ALL, 'C');
  p('あ');
}
