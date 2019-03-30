<?hh

class C {}

<<__Rx>>
function test() {
  class_alias('C', 'D'); // AliasCls
  // we don't emit DefTypeAlias outside of pseudo-mains

  include_once 'defines-includes.inc';
  include      'defines-includes.inc';
  require_once 'defines-includes.inc';
  require      'defines-includes.inc';
  // we don't emit ReqDoc unless the require_once path is relative to
  // the doc root, but we can't predict what path this test runs from
}

<<__EntryPoint>>
function main() {
  test();
  echo "Done\n";
}
