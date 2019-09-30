<?hh

namespace xhp {
	class :foo-element {}

	class :bar {
		attribute :foo-element;
		public function genFoo(:foo-element $e): :foo-element {
			return <foo-element />;
		}
	}
}

namespace {
	function test(:xhp:foo-element $e): :xhp:foo-element {
		return <xhp:foo-element />;
	}
}
