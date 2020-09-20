<?hh
class foo {
  function __construct($this) {
    echo $this."\n";
  }
}
<<__EntryPoint>> function main(): void {
$obj = new foo("Hello world");
}
