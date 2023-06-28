<?hh

function get_vec($vec) :mixed{
  $vec = HH\Lib\Vec\sort($vec);
  return "[".HH\Lib\Str\join($vec, ", ")."]";
}

function print_dict($dict, $level = 1) :mixed{
  $dict = HH\Lib\Dict\sort_by_key($dict);
  foreach ($dict as $k => $v) {
    for ($i = 0; $i < $level; $i++) {
      echo "  ";
    }
    if ($v is vec) {
      echo "$k => ". get_vec($v) . "\n";
    } else {
      echo "$k\n";
      print_dict($v, $level + 1);
    }
  }
}

<<__EntryPoint>>
function main() :mixed{
  echo "Packages\n";
  print_dict(HH\get_all_packages());
  echo "Deployments\n";
  print_dict(HH\get_all_deployments());
}
