<?hh

class A_SQLite3Stmt extends SQLite3Stmt {
  public $___x;
}
<<__EntryPoint>> function main(): void {
  include __DIR__."/builtin_extensions.inc";
  test("SQLite3Stmt", new SQLite3(":memory:"), "CREATE TABLE test (column);");
}
