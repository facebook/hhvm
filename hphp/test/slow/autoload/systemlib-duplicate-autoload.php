<?hh

/**
 * If your PHP/Hack code needs to support multiple versions of HHVM/PHP, the
 * autoloader can be (ab)used to provide an alternative implementation for
 * versions that don't define a builtin.
 *
 * This is also useful if a builtin is renamed - a wrapper class/function can
 * be added.
 */
function main() {
  HH\autoload_set_paths(
    darray[
      'class' => darray[
        'ziparchive' => 'systemlib-duplicate-autoload-1.inc',
        'herpderp' => 'systemlib-duplicate-autoload-2.inc',
      ],
      'function' => darray[
        'get_class' => 'systemlib-duplicate-autoload-1.inc',
        'herp_derp' => 'systemlib-duplicate-autoload-3.inc',
      ],
      'constant' => darray[
        'HPHP_VERSION' => 'systemlib-duplicate-autoload-1.inc',
        'HERP_DERP' => 'systemlib-duplicate-autoload-4.inc',
      ],
    ],
    __DIR__.'/'
  );
  $obj = new ZipArchive();
  $rc = new ReflectionClass($obj);
  var_dump($rc->isUserDefined());

  $obj = new HerpDerp();
  $rc = new ReflectionClass($obj);
  var_dump($rc->isUserDefined());

  $rf = new ReflectionFunction('get_class');
  var_dump($rf->isUserDefined());

  $rf = new ReflectionFunction('herp_derp');
  var_dump($rf->isUserDefined());

  var_dump(HPHP_VERSION);
  var_dump(HERP_DERP);
}


<<__EntryPoint>>
function main_systemlib_duplicate_autoload() {
main();
}
