<?hh // strict

function test(): void {
  var_dump(is_null(5));
  var_dump(is_null(null));
  var_dump(is_string(5));
  var_dump(is_int(5));

  $x = array(1,2,3);
  var_dump(idx($x, 0));
  var_dump(idx($x, 4));
  var_dump(idx($x, 0, 10));
  var_dump(idx($x, 4, 10));
}
