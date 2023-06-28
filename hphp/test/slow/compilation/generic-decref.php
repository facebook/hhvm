<?hh

class X {
}

abstract final class ThingStatics {
  public static $s = 0;
}
function thing() :mixed{
  if (!(ThingStatics::$s++ % 100)) return new X;
  return 42;
}

function test() :mixed{
  thing();
}

<<__EntryPoint>>
function main_generic_decref() :mixed{
  for ($i = 0; $i < 101; $i++) {
    test();
  }
  var_dump(HH\objprof_get_data());
}
