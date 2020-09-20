<?hh
<<__EntryPoint>>
function main_entry(): void {
  echo __DIR__ . "\n";
  echo dirname(__FILE__) . "\n";
  include 'fixtures/folder1/fixture.php';
  folder1_fixture();
  include 'fixtures/folder2/fixture.php';
  folder2_fixture();
  include 'fixtures/folder3/fixture.php';
  folder3_fixture();
  include 'fixtures/folder4/fixture.php';
  folder4_fixture();
}
