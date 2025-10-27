# Methods

Remember, all XHP Objects derive from the [`\Facebook\XHP\Core\node`](/hack/XHP/interfaces) base class, which has some public methods that can be called.

Method | Description
--------|------------
`toStringAsync(): Awaitable<string>` | Renders the element to a string for output. Mutating methods like `setAttribute` can no longer be called after this.
`appendChild(mixed $child): this` | Adds `$child` to the end of the XHP object's array of children. If `$child` is an array, each item in the array will be appended.
`getAttribute(string $name): mixed` | Returns the value of the XHP object's attribute named `$name`. If the attribute is not set, `null` is returned, unless the attribute is required, in which case `AttributeRequiredException` is thrown. If the attribute is not declared or does not exist, then `AttributeNotSupportedException` is thrown. If the attribute you are reading is statically known, use `$this->:name` style syntax instead for better typechecker coverage.
`getAttributes(): dict<string, mixed>` | Returns the XHP object's array of attributes.
`getChildren(): vec<XHPChild>` | Returns the XHP object's children.
`getChildrenOfType<T as XHPChild>(): vec<T>` | Returns the XHP object's children of the specified type (usually a class/interface, but can also be `string` or another type).
`getFirstChild(): ?XHPChild` | Returns the XHP object's first child or `null` if it has no children.
`getFirstChildx(): XHPChild` | Same but throws if the XHP object has no children.
`getFirstChildOfType<T as XHPChild>(): ?T` | Returns the first of XHP object's children of the specified type, or `null` if it has no such children.
`getFirstChildOfTypex<T as XHPChild>(): T` | Same but throws if the XHP object has no children of the specified type.
`getLastChild(): ?XHPChild` | Analogous to `getFirstChild`.
`getLastChildx(): XHPChild` | Analogous to `getFirstChildx`.
`getLastChildOfType<T as XHPChild>(): ?T` | Analogous to `getFirstChildOfType`.
`getLastChildOfType<T as XHPChild>(): T` | Analogous to `getFirstChildOfTypex`.
`isAttributeSet(string $name): bool` | Returns whether the attribute with name `$name` is set.
`replaceChildren(XHPChild ...$children): this` | Replaces all the children of this XHP Object with the variable number of children passed to this method.
`setAttribute(string $name, mixed $val): this` | Sets the value of the XHP object's attribute named `$name`. This does no validation, attributes are only validated when retrieved using `getAttribute` or during rendering.
`setAttributes(KeyedTraversable<string, mixed> $attrs): this` | Replaces the XHP object's array of attributes with `$attrs`.

```hack no-extract
use namespace Facebook\XHP\Core as x;
use type Facebook\XHP\HTML\{li, p, ul};

function build_list(vec<string> $names): x\node {
  $list = <ul id="names" />;
  foreach ($names as $name) {
    $list->appendChild(<li>{$name}</li>);
  }
  return $list;
}

<<__EntryPoint>>
async function xhp_object_methods_run(): Awaitable<void> {
  $names = vec['Sara', 'Fred', 'Josh', 'Scott', 'Paul', 'David', 'Matthew'];

  foreach (build_list($names)->getChildren() as $child) {
    $child as x\node;
    echo 'Child: '.await $child->toStringAsync()."\n";
  }

  echo 'First child: '.
    await (build_list($names)->getFirstChild() as x\node->toStringAsync())."\n";

  echo 'Last child: '.
    await (build_list($names)->getLastChild() as x\node->toStringAsync())."\n";

  foreach (build_list($names)->getAttributes() as $name => $value) {
    echo 'Attribute '.$name.' = '.$value as string."\n";
  }

  echo 'ID: '.build_list($names)->getAttribute('id') as string."\n";
}
```
