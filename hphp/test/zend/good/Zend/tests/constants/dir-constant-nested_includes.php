<?hh
<<__EntryPoint>>
function main_entry(): void {
  echo __DIR__ . "\n";
  echo dirname(__FILE__) . "\n";
  include 'fixtures/folder1/fixture.php';
  folder1_fixture();
  include 'fixtures/folder1/subfolder1/fixture.php';
  folder1_sub1_fixture();
  include 'fixtures/folder1/subfolder2/fixture.php';
  folder1_sub2_fixture();
  include 'fixtures/folder1/subfolder3/fixture.php';
  folder1_sub3_fixture();
  include 'fixtures/folder1/subfolder4/fixture.php';
  folder1_sub4_fixture();
  include 'fixtures/folder2/fixture.php';
  folder2_fixture();
  include 'fixtures/folder2/subfolder1/fixture.php';
  folder2_sub1_fixture();
  include 'fixtures/folder2/subfolder2/fixture.php';
  folder2_sub2_fixture();
  include 'fixtures/folder2/subfolder3/fixture.php';
  folder2_sub3_fixture();
  include 'fixtures/folder2/subfolder4/fixture.php';
  folder2_sub4_fixture();
  include 'fixtures/folder3/fixture.php';
  folder3_fixture();
  include 'fixtures/folder3/subfolder1/fixture.php';
  folder3_sub1_fixture();
  include 'fixtures/folder3/subfolder2/fixture.php';
  folder3_sub2_fixture();
  include 'fixtures/folder3/subfolder3/fixture.php';
  folder3_sub3_fixture();
  include 'fixtures/folder3/subfolder4/fixture.php';
  folder3_sub4_fixture();
  include 'fixtures/folder4/fixture.php';
  folder4_fixture();
  include 'fixtures/folder4/subfolder1/fixture.php';
  folder4_sub1_fixture();
  include 'fixtures/folder4/subfolder2/fixture.php';
  folder4_sub2_fixture();
  include 'fixtures/folder4/subfolder3/fixture.php';
  folder4_sub3_fixture();
  include 'fixtures/folder4/subfolder4/fixture.php';
  folder4_sub4_fixture();
}
