<?hh

namespace test;

type HandlerFunction = (function(AnyArray, ?string, Map<string, mixed>):mixed);

function t(HandlerFunction $f) :mixed{
  $f(varray[], "", Map {});
}

<<__EntryPoint>>
function main_typedef_namespace() :mixed{
t(function($a,$b,$c) {
  echo "Ok";
});
}
