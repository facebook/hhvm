<?hh

// Special handling of a few types.
function t($val) {
  var_dump($val[] = 1);
}

function main() {
  $b = 4;
  $b[5] = 6;
  var_dump($b);

  $c = array(true);
  $c[0][] = 7;
  var_dump($c);

  $s = "abc";
  $s[-1] = "x";
  $s[6] = "gx";
  $s[4] = "ex";
  var_dump($s);

  # t("nonempty"); # Causes fatal.
  t(true);
}
main();
