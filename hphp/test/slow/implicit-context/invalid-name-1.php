<?hh

<<__EntryPoint>>
function main() {
  try {
    HH\ImplicitContext\_Private\create_implicit_context('_Context', 1);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
