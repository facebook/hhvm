<?hh
<<__EntryPoint>>
function main_entry(): void {
  echo __DIR__ . "\n";
  echo dirname(__FILE__) . "\n";
  include 'fixtures/folder1/fixture.php';
  include 'fixtures/folder2/fixture.php';
  include 'fixtures/folder3/fixture.php';
  include 'fixtures/folder4/fixture.php';
}
