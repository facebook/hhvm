<?hh

function main() :mixed{
  $to_open = vec[
    vec['php://stdin', 'r'],
    vec['php://stdout', 'w'],
    vec['php://stderr', 'w'],
    vec['php://fd/1', 'w'], // stdout
    vec['php://temp', 'rw+'],
    vec['php://memory', 'rw+'],
    vec['php://input', 'r'],
    vec['php://output', 'w'],
  ];
  foreach ($to_open as $target) {
    $stream = call_user_func_array(fopen<>, $target);
    $metadata = stream_get_meta_data($stream);
    var_dump(
      dict[
        'target' => $target[0],
        'wrapper' => $metadata['wrapper_type'],
        'stream' => $metadata['stream_type'],
      ]
    );
  }
}


<<__EntryPoint>>
function main_php_wrapper_stream_types() :mixed{
main();
}
