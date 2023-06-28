<?hh

function main() :mixed{
  $to_open = varray[
    varray['php://stdin', 'r'],
    varray['php://stdout', 'w'],
    varray['php://stderr', 'w'],
    varray['php://fd/1', 'w'], // stdout
    varray['php://temp', 'rw+'],
    varray['php://memory', 'rw+'],
    varray['php://input', 'r'],
    varray['php://output', 'w'],
  ];
  foreach ($to_open as $target) {
    $stream = call_user_func_array(fopen<>, $target);
    $metadata = stream_get_meta_data($stream);
    var_dump(
      darray[
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
