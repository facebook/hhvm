<?hh
function autoload_miss($str1, $str2) {
  echo "Failure handler called: $str1 $str2\n";
}

<<__EntryPoint>>
function main_autoload_type_alias_bug() {
HH\autoload_set_paths(
  darray[
    'class' => varray[],
    'constant' => varray[],
    'function' => varray[],
    'failure' => 'autoload_miss',
    'type' => darray['foo' => 'autoload-type-alias-bug-2.inc'],
  ],
  __DIR__.'/'
);
include 'autoload-type-alias-bug-1.inc';
echo "Done\n";
}
