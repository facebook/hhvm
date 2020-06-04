<?hh

namespace test;

class foo {
	public function __construct() {
		\var_dump(__METHOD__);
		\var_dump(__CLASS__);
		\var_dump(__FUNCTION__);
	}
}
<<__EntryPoint>>
function entrypoint_constants_006(): void {

  \var_dump(__DIR__);
  \var_dump(__FILE__);
  \var_dump(__LINE__);

  new foo;

  \var_dump(__NAMESPACE__);
}
