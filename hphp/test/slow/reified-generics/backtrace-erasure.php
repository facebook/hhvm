<?hh

function f<reify Ta, Tb>() :mixed{
  try {
    throw new Exception();
  } catch (Exception $e) {
    echo $e->getTraceAsString() . "\n";
  }
}
function g<reify Ta, Tb>() :mixed{ f<Ta, Tb>(); }
function h<reify Ta, Tb>() :mixed{ g<Ta, Tb>(); }

function a<T>() :mixed{
  g<int, T>();
}
<<__EntryPoint>>
function entrypoint_backtraceerasure(): void {

  a();
}
