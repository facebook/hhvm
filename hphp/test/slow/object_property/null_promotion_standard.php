<?hh

<<__EntryPoint>>
function main() {
  try {
    $herp->derp = 'foobar';
  } catch (Exception $e) {
    echo $e->getMessage();
  }
}
