<?hh

function ctype_test_002($function) :mixed{
	$n1 = $n2 = $n3 = 0;
	// test portable POSIX characters 0..127
	for ($a=0;$a<128;$a++) {
		$c = chr($a);
		if(HH\dynamic_fun($function)($a)) $n1++;
		if(HH\dynamic_fun($function)("$c$c$c")) $n2++;
		if(HH\dynamic_fun($function)("1-$c$c$c-x")) $n3++;
	}
	print "$function $n1 $n2 $n3\n";
}

<<__EntryPoint>>
function main_entry(): void {

  setlocale(LC_ALL,"C");
  print "LOCALE is '" . setlocale(LC_ALL,0) . "'\n";

  ctype_test_002("ctype_lower");
  ctype_test_002("ctype_upper");
  ctype_test_002("ctype_alpha");
  ctype_test_002("ctype_digit");
  ctype_test_002("ctype_alnum");
  ctype_test_002("ctype_cntrl");
  ctype_test_002("ctype_graph");
  ctype_test_002("ctype_print");
  ctype_test_002("ctype_punct");
  ctype_test_002("ctype_space");
  ctype_test_002("ctype_xdigit");
}
