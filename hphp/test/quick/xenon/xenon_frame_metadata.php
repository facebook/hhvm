<?hh

function bar($name) {
  echo "calling bar($name)\n";
  // do something silly to force existence of bar() frame
  call_user_func('debug_backtrace');
}

function foo(string $name) {
  HH\set_frame_metadata($name);
  bar($name);
}

function main() {
  foo('hello');
  foo('world');
}

main();

$last = null;
foreach (xenon_get_data() as $sample) {
  foreach ($sample['phpStack'] as $frame) {
    if (array_key_exists('metadata', $frame)) {
      $key = $frame['function'].' '.$frame['metadata'];
      if ($key !== $last) {
        echo "$key\n";
        $last = $key;
      }
    }
  }
}
