<?hh // strict

namespace NS_nowdoc_string_literals;

function main(): void {
  $v = 123;

// test using unescaped ", embedded (actual) tab, variable substitution, multiple lines

  $s = <<<	  'ID'
S'o'me "\"t e\txt; \$v = $v"
Some more text
ID;
  echo ">$s<\n\n";

  var_dump(<<<'X'
X
);

  var_dump(<<<'X'
xxx
yyy
X
);
}

/* HH_FIXME[1002] call to main in strict*/
main();
