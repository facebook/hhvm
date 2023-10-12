<?hh // strict

namespace first {
  xhp class firstxhp extends \XHPTest implements \XHPChild {}
}
namespace second {
  xhp class foo extends \first\firstxhp {}
}
namespace third {
  xhp class bar extends \first\firstxhp {}
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
      <:second:foo>{varray[vec[<:third:bar />]]}</:second:foo>
    </:third:bar>;
  }
}
