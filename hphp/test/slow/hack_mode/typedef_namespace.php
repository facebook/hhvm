<?hh

namespace test;

type HandlerFunction = (function(array, ?string, Map<string, mixed>):mixed);

function t(HandlerFunction $f) {
  $f([], "", Map {});
}
t(function($a,$b,$c) {
  echo "Ok";
});
