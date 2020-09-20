<?hh
<<__EntryPoint>>
function entrypoint_autoload2(): void {

  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'c' => 'autoload1.inc',
      ],
    ],
    __DIR__.'/',
  );

  $arr = varray["C"];
  $obj = new $arr[0];

  echo 'Done!';
}
