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
}
