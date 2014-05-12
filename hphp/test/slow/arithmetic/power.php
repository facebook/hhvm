<?php

function main() {
  var_dump(0 ** 0);
  var_dump(0 ** 1);
  var_dump(1 ** 0);
  var_dump(1 ** 1);

  var_dump(2 ** 2);
  var_dump(2 ** 3);
  var_dump(2 ** 4);
  var_dump(2 ** 5);
  var_dump(2 ** 6);

  var_dump(10 ** 10);

  var_dump(0.0 ** 0.1);
  var_dump(0.2 ** 1.3);
  var_dump(1.4 ** 0.5);
  var_dump(1.6 ** 1.7);

  var_dump(2.8 ** 2.9);
  var_dump(2.1 ** 3.2);
  var_dump(2.3 ** 4.4);
  var_dump(2.5 ** 5.6);
  var_dump(2.7 ** 6.8);

  var_dump(10.9 ** 10.9);

  var_dump(0 ** 0.1);
  var_dump(0 ** 1.3);
  var_dump(1 ** 0.5);
  var_dump(1 ** 1.7);

  var_dump(2 ** 2.9);
  var_dump(2 ** 3.2);
  var_dump(2 ** 4.4);
  var_dump(2 ** 5.6);
  var_dump(2 ** 6.8);

  var_dump(10 ** 10.9);
  var_dump(2 ** 64);
}

main();
