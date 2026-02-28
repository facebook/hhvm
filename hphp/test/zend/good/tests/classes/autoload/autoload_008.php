<?hh

function f() :mixed{
  throw new Exception();
}

<<__EntryPoint>> function autoload_008(): void {
  try {
    f();
  } catch (UndefC $u) {
    echo "In UndefClass catch block.\n";
  } catch (Exception $e) {
    echo "In Exception catch block. Autoload should not have been triggered.\n";
  }
}
