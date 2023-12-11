<?hh


class someclass {
  static public function yo() :mixed{
    return $_ENV['hey'] ?? false;
  }
}

function asd() :mixed{ return mt_rand() ? 'a' : 2; }
function main() :mixed{
  $time = asd();
  $items = vec[];
  if (someclass::yo()) {
    $items[] = 'ZZZZ';
  }
  $items[] = $time;
  return $items;
}

<<__EntryPoint>>
function main_array_possibly_empty_001() :mixed{
main();
}
