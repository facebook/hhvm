<?hh

<<__EntryPoint>>
function main(): void {
  setlocale(LC_ALL,"C");
  $tests = dict[
    "ctype_lower" => ctype_lower<>,
    "ctype_upper" => ctype_upper<>,
    "ctype_alpha" => ctype_alpha<>,
    "ctype_digit" => ctype_digit<>,
    "ctype_alnum" => ctype_alnum<>,
    "ctype_cntrl" => ctype_cntrl<>,
    "ctype_graph" => ctype_graph<>,
    "ctype_print" => ctype_print<>,
    "ctype_punct" => ctype_punct<>,
    "ctype_space" => ctype_space<>,
    "ctype_xdigit" => ctype_xdigit<>,
  ];
  foreach ($tests as $k => $v) {
    $n = 0;
    for ($a = 0; $a < 256; $a++) {
      if ($v($a)) $n++;
    }
    echo "$k $n\n";
  }
}
