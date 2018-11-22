<?hh

function f<reify Ta, Tb>() {
  try {
    throw new Exception();
  } catch (Exception $e) {
    echo $e->getTraceAsString() . "\n";
  }
}
function g<reify Ta, Tb>() { f<reify Ta, Tb>(); }
function h<reify Ta, Tb>() { g<reify Ta, Tb>(); }

g<reify int, string>();
