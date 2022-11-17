<?hh

function autoload_miss($str1, $str2) {
  echo "Failure handler called: $str1 $str2\n";
}

<<__EntryPoint>>
function main_requireclass(): void {
  echo (new C())->bar() . "\n";
  echo "done\n";
}
