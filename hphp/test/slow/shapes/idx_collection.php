<?hh

<<__EntryPoint>>
function main_idx() {
  $empty_map = new Map();
  var_dump(Shapes::idx($empty_map, 'a'));
  var_dump(Shapes::idx($empty_map, 'a', 2));
  var_dump(Shapes::idx($empty_map, 4));
  var_dump(Shapes::idx($empty_map, 4, 5));

  $str_map = new Map(dict['a' => 3]);
  var_dump(Shapes::idx($str_map, 'a'));
  var_dump(Shapes::idx($str_map, 'a', 2));

  $int_map = new Map(dict[4 => 'x']);
  var_dump(Shapes::idx($int_map, 4));
  var_dump(Shapes::idx($int_map, 4, 5));

  $empty_vector = new Vector();
  var_dump(Shapes::idx($empty_vector, 0));
  var_dump(Shapes::idx($empty_vector, 0, 2));
  var_dump(Shapes::idx($empty_vector, 4));
  var_dump(Shapes::idx($empty_vector, 4, 5));

  $a_vector = new Vector(vec['a']);
  var_dump(Shapes::idx($a_vector, 0));
  var_dump(Shapes::idx($a_vector, 0, 2));
  var_dump(Shapes::idx($a_vector, 4));
  var_dump(Shapes::idx($a_vector, 4, 5));
}
