<?hh

// Test iterating over a ImmVector with a "foreach".

function main() :mixed{

  $fv = ImmVector {1, 2, 3};

  foreach ($fv as $e) {
    var_dump($e);
  }

}

<<__EntryPoint>>
function main_foreach() :mixed{
main();
}
