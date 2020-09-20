<?hh

function test($a){
  echo "$a\n";
}

<<__EntryPoint>>
function main_1512() {
test(1, test(2), test(3, test(4), test(5)));
}
