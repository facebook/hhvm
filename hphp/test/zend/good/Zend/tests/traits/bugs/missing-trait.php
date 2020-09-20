<?hh

class TraitsTest {
  use THello;
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$test = new TraitsTest();
}
