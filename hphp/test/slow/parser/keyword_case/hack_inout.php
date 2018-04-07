<?hh

Function takes_inout(InOut $x) {
  $x .= " INOUT";
}

Function test_inout() {
  $x = "THE OLD";
  takes_inout(INOUT $x);
  ECHO $x;
}

test_inout();
