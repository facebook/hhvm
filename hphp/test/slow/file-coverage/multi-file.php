<?hh

function handle_autoload($kind, $name) {
  echo "In handle_autoload($kind, $name)\n";
  if ($kind === 'function' && $name === 'B_foo') {
    require __DIR__.'/multi-file-b.inc';
  }
  if ($kind === 'function' && $name === 'C_foo') {
    require __DIR__.'/multi-file-c.inc';
  }
}

function print_cover_maps(dict<string, vec<int>> $map) {
  foreach ($map as $path => $lines) {
    if ($lines[0] === 1) {
      // Don't count the pseudomain
      array_shift(inout $lines);
    }
    if (count($lines) === 0) continue;
    $file = basename($path);
    $first = -1;
    $last = -1;
    echo "$file:";
    foreach ($lines as $line) {
      if ($last + 1 !== $line) {
        if ($last !== $first)  echo "-$last\n$file:";
        else if ($last !== -1) echo "\n$file:";
        echo "$line";
        $first = $line;
      }
      $last = $line;
    }
    if ($first !== $line) echo "-$line\n";
    else                  echo "\n";
  }
}

<<__EntryPoint>>
function main() {
  HH\autoload_set_paths(
    darray[
      'class' => varray[],
      'function' => darray['a_foo' => __DIR__.'/multi-file-a.inc'],
      'constant' => varray[],
      'type' => varray[],
      'failure' => handle_autoload<>,
    ],
    __DIR__
  );

  A_foo(false); A_foo(false);
  B_foo();      B_foo();
  HH\enable_per_file_coverage(keyset[
    __DIR__.'/multi-file-a.inc',
    __DIR__.'/multi-file-c.inc'
  ]);
  A_foo(12); A_foo(12);
  B_foo();   B_foo();
  C_foo();   C_foo();
  print_cover_maps(HH\get_all_coverage_data());
}
