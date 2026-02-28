<?hh

function test($a):mixed{
  echo "$a\n";
}

<<__EntryPoint>>
function main_1512() :mixed{
test(1, test(2), test(3, test(4), test(5)));
}
