<?hh

// force existence of bar() frame
<<__NEVER_INLINE>>
function bar($name) :mixed{
  echo "calling bar($name)\n";
}

function foo(string $name) :mixed{
  HH\set_frame_metadata($name);
  bar($name);
}

function main() :mixed{
  foo('hello');
  foo('world');
}
<<__EntryPoint>> function main_entry(): void {
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
