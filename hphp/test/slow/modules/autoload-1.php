<?hh

<<__EntryPoint>>
function main() {
  HH\autoload_set_paths(
    dict[
      'module' => darray[
        'a' => 'autoload-1.inc',
        'b' => 'autoload-1.inc'
      ],
      'failure' => (...$args) ==> var_dump($args),
    ],
    __DIR__.'/'
  );
  $m = new ReflectionModule('a');
  var_dump($m->getName());
  try { new ReflectionModule('b'); } catch (Exception $e) { var_dump($e->getMessage()); }
}
