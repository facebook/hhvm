<?hh

function min_max_single($arg) :mixed{
  var_dump(max($arg));
  var_dump(min($arg));
}

function min_max_multi(...$args) :mixed{

  $multi_max = max(...$args);
  $single_max = max($args);
  var_dump($multi_max);
  var_dump($multi_max === $single_max);

  $multi_min = min(...$args);
  $single_min = min($args);
  var_dump($multi_min);
  var_dump($multi_min === $single_min);
}

function single_arg() :mixed{
  echo ' === Single Array ===', "\n";
  min_max_single(vec[3, 1, 6, 7]);
  min_max_single(vec[2, 4, 5]);
  min_max_single(dict[1 => "1236150163"]);

  echo ' === Single Collection ===', "\n";
  min_max_single(Vector {3, 1, 6, 7});
  min_max_single(Set {2, 4, 5});
  min_max_single(Map {1 => "1236150163"});
}

function multi_arg() :mixed{
  echo ' === Multiple Arguments ===', "\n";
  min_max_multi(vec[vec[2], 4, 8], vec[vec[2, 5, 1]]);
  min_max_multi(1, 1.0);
  min_max_multi(1.0, 1);
}


<<__EntryPoint>>
function main_max_and_min() :mixed{
single_arg();
multi_arg();
}
