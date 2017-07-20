<?hh

namespace foo\bar {
  function baz() {
    var_dump(__FUNCTION__);
  }
}

namespace foo {
  class bar {
    public function __construct() {
      var_dump(__CLASS__);
    }
  }

  class baz {
    public function __construct() {
      var_dump(__CLASS__);
    }
  }
}

namespace {

use namespace foo\{bar, baz};
use type foo\{bar, baz};

bar\baz();
new bar();
new baz();

}
