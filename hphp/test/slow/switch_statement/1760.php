<?hh

class X {
}
function test($x) :mixed{
  switch (true) {
    case $x is X: var_dump('X');
 break;
    default: var_dump('Other');
 break;
  }
}

<<__EntryPoint>>
function main_1760() :mixed{
test(new X);
}
