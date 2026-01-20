# XHP Attribute Selection

When working with [XHP](/hack/XHP/introduction), use the `->:` operator to retrieve an XHP class [attribute](/hack/XHP/basic-usage#attributes) value.

The operator can also be used on arbitrary expressions that resolve to an XHP object (e.g. `$a ?? $b)->:`).

```hack no-extract
use namespace Facebook\XHP\Core as x;

final xhp class user_info extends x\element {
  attribute int userid @required;
  attribute string name = "";

  protected async function renderAsync(): Awaitable<x\node> {
    return
      <x:frag>User with id {$this->:userid} has name {$this->:name}</x:frag>;
  }
}
```
