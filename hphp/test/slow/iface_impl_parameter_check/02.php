<?php // should fail

interface I {
  function query(); // HipHop syntax enabled => full check
}

class C extends PDO implements I {
}

print(print_r(ini_get_all()));

print("Success\n");
