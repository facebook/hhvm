<?hh

function main() {
  print(
    '$undef->foo should warn ("Creating default object from default value"):'.
    "\n"
  );
  $herp->derp = 'foobar';
  var_dump($herp);
}


<<__EntryPoint>>
function main_null_promotion_hh() {
main();
}
