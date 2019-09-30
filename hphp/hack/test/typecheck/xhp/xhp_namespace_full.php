<?hh // strict

namespace first {
	class :xhp implements \XHPChild {}
}
namespace second {
	class :foo extends \first\:xhp {}
}
namespace third {
	class :bar extends \first\:xhp {}
}
namespace fourth {
	class A {
		public function __toString(): string {
			return 'A';
		}
	}
}
namespace fifth {
	function test(): void {
		<:second:foo />;
		<:second:foo></:second:foo>;
		<:second:foo>
			<:third:bar />
		</:second:foo>;
		<:second:foo>
			{null}
		</:second:foo>;
		<:second:foo>
			{3}
		</:second:foo>;
		<:second:foo>test!</:second:foo>;
		<:second:foo>
			{new \fourth\A()}
		</:second:foo>;
		<:third:bar>
			<:second:foo>{array(vec[<:third:bar />])}</:second:foo>
		</:third:bar>;
	}
}
