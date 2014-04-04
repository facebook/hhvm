<?php

function main() {
  print(
    '$undef->foo should warn ("Creating default object from default value"):'.
    "\n"
  );
  $herp->derp = 'foobar';
  var_dump($herp);
  print('As should $undef->foo["bar"]:'."\n");
  $foo->bar['baz'] = 'herpderp';
  var_dump($foo);
}

main();
