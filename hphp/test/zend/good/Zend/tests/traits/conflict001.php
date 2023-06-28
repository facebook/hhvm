<?hh

trait THello1 {
  private function hello() :mixed{
    echo 'Hello';
  }
}

trait THello2 {
  private function hello() :mixed{
    echo 'Hello';
  }
}

class TraitsTest {
	use THello1;
	use THello2;
}
<<__EntryPoint>>
function entrypoint_conflict001(): void {
  error_reporting(E_ALL);
}
