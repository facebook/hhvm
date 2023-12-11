<?hh

<<__EntryPoint>>
function main() :mixed{
  $a = dict[
    "10" => "string_A",
    10   => "int_A",
  ];
  $b = dict[
    10   => "int_B",
    "10" => "string_B",
    ];
  $a_dict = dict($a);
  $b_dict = dict($b);
  var_dump($a_dict);
  var_dump($b_dict);
  echo "\n---- A alone ----\n";
  var_dump(array_merge($a));
  var_dump(array_merge($a_dict));
  echo "\n---- B alone ----\n";
  var_dump(array_merge($b));
  var_dump(array_merge($b_dict));
  echo "\n---- A first ----\n";
  var_dump(array_merge($a, $b));
  var_dump(array_merge($a_dict, $b_dict));
  echo "\n---- B first ----\n";
  var_dump(array_merge($b, $a));
  var_dump(array_merge($b_dict, $a_dict));
}
