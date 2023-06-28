<?hh

<<__EntryPoint>>
function main() :mixed{
  try {
    $herp->derp = 'foobar';
  } catch (Exception $e) {
    echo $e->getMessage();
  }
}
