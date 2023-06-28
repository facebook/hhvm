<?hh

function bar($name) :mixed{
  echo "calling bar($name)\n";
  // do something silly to force existence of bar() frame
  call_user_func(debug_backtrace<>);
}

function foo(string $name) :mixed{
  HH\set_frame_metadata($name);
  bar($name);
}

function main() :mixed{
  foo('hello');
  foo('world');
}


<<__EntryPoint>>
function main_xenon_frame_metadata() :mixed{
main();

$last = null;
foreach (xenon_get_data() as $sample) {
  foreach ($sample['stack'] as $frame) {
    if (array_key_exists('metadata', $frame)) {
      $key = $frame['function'].' '.$frame['metadata'];
      if ($key !== $last) {
        echo "$key\n";
        $last = $key;
      }
    }
  }
}
}
