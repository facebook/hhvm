<?hh

class A {
  <<__LateInit>> public int $x;
}

class B extends A {}

<<__EntryPoint>> function test() :mixed{
  echo "---- serialize ----\n";
  try {
    var_dump(serialize(new B()));
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  echo "---- json_encode ----\n";
  try {
    var_dump(json_encode(new B()));
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  echo "---- var_dump ----\n";
  try {
    var_dump(new B());
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  echo "---- var_export ----\n";
  try {
    var_export(new B());
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
