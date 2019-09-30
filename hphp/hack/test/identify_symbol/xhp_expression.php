<?hh

namespace xhp {
	class :foo-element {}
}

namespace {
	function test() {
		<xhp:foo-element />;
	}
}
