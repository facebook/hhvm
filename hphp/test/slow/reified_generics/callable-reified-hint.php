<?hh

function f<reify T>(T $x) { echo "done\n"; }
function g((function(mixed...):void) $x) { echo "done\n"; }

<<__EntryPoint>>
function main() {
  g(() ==> 1);
  g(1);
  f<(function(mixed...):void)>(1);
  f<(function(mixed...):void)>(() ==> 1);
}
