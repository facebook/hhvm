<?hh

function f() {
  throw new Exception();
}

<<__EntryPoint>> function main(): void {
  try {
    f();
  } catch (UndefC $u) {
    echo "In UndefClass catch block.\n";
  } catch (Exception $e) {
    echo "In Exception catch block. Autoload should not have been triggered.\n";
  }
}
