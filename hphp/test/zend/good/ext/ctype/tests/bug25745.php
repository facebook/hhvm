<?hh

<<__EntryPoint>>
function main(): void {
  $funcs = dict[
    "ctype_alnum" => ctype_alnum<>,
    "ctype_alpha" => ctype_alpha<>,
    "ctype_cntrl" => ctype_cntrl<>,
    "ctype_digit" => ctype_digit<>,
    "ctype_graph" => ctype_graph<>,
    "ctype_lower" => ctype_lower<>,
    "ctype_print" => ctype_print<>,
    "ctype_punct" => ctype_punct<>,
    "ctype_space" => ctype_space<>,
    "ctype_upper" => ctype_upper<>,
    "ctype_xdigit" => ctype_xdigit<>,
  ];

  foreach ($funcs as $name => $ctype_func) {
    for ($i = 0; $i < 256; $i++) {
      $a = $ctype_func($i);
      $b = $ctype_func(chr($i));
      if ($a != $b) {
        echo "broken... $name($i) = $a, $name(chr($i)) = $b\n";
        exit(1);
      }
    }
  }

  echo "ok\n";
}
