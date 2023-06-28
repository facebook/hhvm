<?hh

namespace foo\bar {
  function baz() :mixed{
    \var_dump(__LINE__);
  }
}

namespace foo {
  class bar {
    public function __construct() {
      \var_dump(__LINE__);
    }
  }
}

namespace {

use namespace foo\bar;
<<__EntryPoint>> function main(): void {
bar\baz();
new bar();

}
}
