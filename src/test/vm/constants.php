<?php

function f() {
  print "Enter f()\n";
  var_dump(CN);
  var_dump(CB);
  var_dump(CI);
  var_dump(CD);
  var_dump(CS);
  print "Leave f()\n";
}

function main() {
  print "Test begin\n";

  var_dump(CN);
  var_dump(CB);
  var_dump(CI);
  var_dump(CD);
  var_dump(CS);
  f();
  var_dump(CN);
  var_dump(CB);
  var_dump(CI);
  var_dump(CD);
  var_dump(CS);
  f();
#===============================================================================
  define('CN', null);
  define('CB', true);
  define('CI', 42);
  define('CD', 42.31);
  define('CS', "--- CS ---");
#===============================================================================
  var_dump(CN);
  var_dump(CB);
  var_dump(CI);
  var_dump(CD);
  var_dump(CS);
  f();
  var_dump(CN);
  var_dump(CB);
  var_dump(CI);
  var_dump(CD);
  var_dump(CS);
  f();
  print "Test end\n";
}

main();
