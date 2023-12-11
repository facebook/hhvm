<?hh


class someclass {
  static public function yo() :mixed{
    return $_ENV['hey'] ?? false;
  }
}

function asd() :mixed{ return mt_rand() ? 'a' : 2; }
function array_of_one() :mixed{ return vec[asd()]; }
function main() :mixed{
  $time = asd();
  $items = dict[];
  if (someclass::yo()) {
    $items = array_of_one();
  }
  $items[1] = $time;
  return $items;
}

<<__EntryPoint>>
function main_array_possibly_empty_003() :mixed{
main();
}
