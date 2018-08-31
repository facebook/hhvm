<?php

function test_and_or_xor() {
  var_dump(TRUE OR DIE('should not get here'));
  var_dump(FALSE AND DIE('should not get here'));
  var_dump(TRUE XOR FALSE);
}

function test_if_elseif_else($x) {
  IF ($x === 0) {
    ECHO "NONE\n";
  } ELSEif ($x === 1) {
    ECHO "ONE\n";
  } ELSE IF ($x === 2) {
    ECHO "TWO\n";
  } eLsE {
    ECHO "MANY\n";
  }
}

function test_for() {
  FOR ($i = 0; $i < 10; $i += 3) {
    var_dump($i);
  }
}

function test_foreach() {
  forEACH (ARRAY("one", "two", "three") AS $n) {
    var_dump($n);
  }
}

function test_while() {
  $i = true;
  While ($i) {
    ECHO "IN WHILE\n";
    $i = false;
  }
}

function test_do_while() {
  $i = false;
  DO {
    ECHO "IN DO WHILE\n";
  } WhilE ($i);
}

function test_switch($x) {
  Switch ($x) {
    CASE 0: ECHO "NONE\n"; BREAK;
    Case 1: ECHO "ONE\n"; Break;
    casE 2: ECHO "TWO\n"; breaK;
    deFAULT: ECHO "MANY\n";
  }
}

function test_break_continue() {
  WHILE (TRUE) {
    ECHO "IN WHILE TRUE JUST ONCE\n";
    BREAK;
  }
  forEACH (array(1, 2) as $x) {
    ECHO "FOREACH PRINT ME $x\n";
    Continue;
    ECHO "DON'T PRINT ME BRO\n";
  }
}

function test_goto() {
  ECHO "WE'RE GOING PLACES\n";
  goTO success;
  ECHO "DON'T PRINT ME BRO\n";
  success:
  ECHO "PLACES!\n";
}

function test_return() {
  RETURN "TO SENDER";
}

function my_generator() {
  YIELD "DIVIDENDS";
  YIELD "RESULTS";
}

function test_yield() {
  ForEach (my_generator() AS $got) {
    ECHO "Got $got\n";
  }
}


<<__EntryPoint>>
function main_control_flow() {
test_and_or_xor();
test_if_elseif_else(0);
test_if_elseif_else(1);
test_if_elseif_else(2);
test_if_elseif_else(3);
test_for();
test_foreach();
test_while();
test_do_while();
test_switch(0);
test_switch(1);
test_switch(2);
test_switch(3);
test_break_continue();
test_goto();
var_dump(test_return());
test_yield();
}
