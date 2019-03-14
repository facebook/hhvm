<?hh

namespace foo\bar {
  function baz() {
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
use foo\bar;

bar\baz();
new bar();

}
