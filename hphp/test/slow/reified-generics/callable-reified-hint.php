<?hh

function f<reify T>(T $x) :mixed{ echo "done\n"; }
function g((function(mixed...):void) $x) :mixed{ echo "done\n"; }

<<__EntryPoint>>
function main() :mixed{
  g(() ==> 1);
  g(1);
  f<(function(mixed...):void)>(1);
  f<(function(mixed...):void)>(() ==> 1);
}
