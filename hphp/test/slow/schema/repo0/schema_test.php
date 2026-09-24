<?hh

<<__EntryPoint>>
function main() {
  var_dump(HH\unit_schema());
  var_dump(HH\unit_schema(__DIR__.'/../repo1'));
  var_dump(HH\unit_schema(__DIR__.'/../repo2'));
  var_dump(HH\unit_schema(__DIR__.'/../repo3'));

  var_dump(HH\unit_schema() === HH\unit_schema(__DIR__.'/../repo1'));
  var_dump(HH\unit_schema() === HH\unit_schema(__DIR__.'/../repo2'));
  var_dump(HH\unit_schema() === HH\unit_schema(__DIR__.'/../repo3'));
  var_dump(HH\unit_schema(__DIR__.'/../repo1') === HH\unit_schema(__DIR__.'/../repo2'));
  var_dump(HH\unit_schema(__DIR__.'/../repo1') === HH\unit_schema(__DIR__.'/../repo3'));
  var_dump(HH\unit_schema(__DIR__.'/../repo2') === HH\unit_schema(__DIR__.'/../repo3'));

  $repo = __DIR__.'/../repo4';
  $default = HH\mangle_unit_sha1('source', '.php', $repo);
  $loose = HH\mangle_unit_sha1(
    'source',
    '.php',
    $repo,
    $repo.'/loose/file.php',
  );
  $strict = HH\mangle_unit_sha1(
    'source',
    '.php',
    $repo,
    $repo.'/strict/file.php',
  );
  var_dump($default === $loose);
  var_dump($default === $strict);
}
