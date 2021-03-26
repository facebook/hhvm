<?hh
<<__EntryPoint>>
function main_entry(): void {
  echo __DIR__ . "\n";
  echo dirname(__FILE__) . "\n";
  include 'fixtures/folder1/fixture.inc';
  folder1_fixture();
  include 'fixtures/folder2/fixture.inc';
  folder2_fixture();
  include 'fixtures/folder3/fixture.inc';
  folder3_fixture();
  include 'fixtures/folder4/fixture.inc';
  folder4_fixture();
}
