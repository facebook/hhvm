<?hh

namespace test;

type HandlerFunction = (function(array, ?string, Map<string, mixed>):mixed);

function t(HandlerFunction $f) {
  $f(varray[], "", Map {});
}

<<__EntryPoint>>
function main_typedef_namespace() {
t(function($a,$b,$c) {
  echo "Ok";
});
}
