<?hh

namespace xhp {
	class :foo-element {}
}

namespace {
	class :xhp:bar-component {
		attribute :xhp:foo-element;
	}
}
