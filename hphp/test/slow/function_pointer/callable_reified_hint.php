<?hh

function f<reify T>(T $x) :mixed{ echo "done\n"; }
function g((function(mixed...):void) $x) :mixed{ echo "done\n"; }

<<__EntryPoint>>
function main() :mixed{
  $g = g<>;
  $g(() ==> 1);
  $g(1);

  $f = f<(function(mixed...):void)>;
  $f(1);
  $f(() ==> 1);
}
