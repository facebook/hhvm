<?hh

<<__ALWAYS_INLINE>>
function test($dict) {
  return idx($dict, rand() % 2);
}

function rand_dict() {
  return dict[0 => rand(), 1 => rand()];
}

<<__EntryPoint>>
function main() {
  for ($i = 0; $i < 100; ++$i) {
    test(rand_dict());
  }

  if (!apc_add('hello', 'world')) {
    echo "boom\n";
    test(dict[0 => 'hello', 1 => 'world']);
  } else {
    echo "profiling\n";
  }
}
