<?hh

function f<reify Ta, Tb>() {
  try {
    throw new Exception();
  } catch (Exception $e) {
    echo $e->getTraceAsString() . "\n";
  }
}
function g<reify Ta, Tb>() { f<Ta, Tb>(); }
function h<reify Ta, Tb>() { g<Ta, Tb>(); }

function a<T>() {
  g<int, T>();
}

a();
