<?hh

function make_num(): num {
  return 10101;
}

function make_arraykey(): arraykey {
  return '10101';
}

function make_mixed(): mixed {
  return 10101;
}

function make_nonnull(): nonnull {
  return 10101;
}

function test(): void {
  $bool = true;
  $int = 10101;
  $float = 10.101;
  $string = '10101';
  $null = null;
  $num = make_num();
  $arraykey = make_arraykey();

  $mixed = make_mixed();
  $nonnull = make_nonnull();

  "$bool";
  ''.$bool;

  "$int";
  ''.$int;

  "$float";
  ''.$float;

  "$string";
  ''.$string;

  "$null";
  ''.$null;

  "$num";
  ''.$num;

  "$arraykey";
  ''.$arraykey;
}
