<?hh

<<__EntryPoint>>
function main() {
  try {
    HH\set_implicit_context('_Context', 1, 'memokey');
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
