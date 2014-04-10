<?php

function main() {
  print(
    '$undef->foo should warn ("Creating default object from default value"):'.
    "\n"
  );
  $herp->derp = 'foobar';
  var_dump($herp);
  print('But $undef->foo["bar"] should not warn:'."\n");
  $foo->bar['baz'] = 'herpderp';
  var_dump($foo);
}

main();
