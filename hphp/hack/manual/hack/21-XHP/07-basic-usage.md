# Basic Usage

XHP is a syntax to create actual Hack objects, called *XHP objects*. They are meant to be used as a tree, where children can either be
other XHP objects or text nodes (or, rarely, other non-XHP objects).

## Creating a Simple XHP Object

Instead of using the `new` operator, creating XHP looks very much like XML:

```hack no-extract
$my_xhp_object = <p>Hello, world</p>;
```

`$my_xhp_object` now contains an instance of the `p` class.
It is a real object, meaning that `is_object` will return `true` and you can call methods on it.

<FbHistorical>

Before XHP namespace support (in XHP-Lib v3), XHP classes
lived in a separate (but still global) namespace from regular classes, denoted
by a `:` prefix in the typechecker and an `xhp_` prefix at runtime. `<p>` would
therefore instantiate a class named `:p` in Hack code and `xhp_p` at runtime. It
would therefore not conflict with a global non-XHP class named `p`, but would
conflict with a class named `xhp_p`.

</FbHistorical>

The following example utilizes three XHP classes: `div`, `strong`, `i`. Whitespace is insignificant, so you can create a readable
tree structure in your code.

```hack no-extract
use type Facebook\XHP\HTML\{div, i, strong};

<<__EntryPoint>>
function basic_usage_examples_basic_xhp(): void {
  \var_dump(
    <div>
      My Text
      <strong>My Bold Text</strong>
      <i>My Italic Text</i>
    </div>,
  );
}
```

The `var_dump` shows that a tree of objects has been created, not an HTML/XML string. An HTML string can be produced by calling `await $xhp_object->toStringAsync()`.

## Namespace Syntax

When instantiating an XHP class using the `<ClassName>` syntax, `:` must be used
instead of `\` as a namespace separator (this mirrors XML's namespace syntax).
These are all equivalent ways to instantiate a `Facebook\XHP\HTML\p` object:

```hack no-extract
use type Facebook\XHP\HTML\p;
$xhp = <p>Hello, world</p>;
```

```hack no-extract
use namespace Facebook\XHP\HTML;
$xhp = <HTML:p>Hello, world</HTML:p>;
```

```hack no-extract
use namespace Facebook\XHP\HTML as h;
$xhp = <h:p>Hello, world</h:p>;
```

```hack no-extract
// exists in the root namespace:
$xhp = <Facebook:XHP:HTML:p>Hello, world</Facebook:XHP:HTML:p>;
```

```hack no-extract
namespace CustomNamespace;  // from any namespace:
$xhp = <:Facebook:XHP:HTML:p>Hello, world</:Facebook:XHP:HTML:p>;
```

In all other contexts, `\` must be used, for example:

```hack no-extract
if ($obj is HTML\p) { ... }
h\p::staticMethod();
$class_name = Facebook\XHP\HTML\p::class;
final xhp class my_element extends \Facebook\XHP\Core\element { ... }
```

<FbHistorical>

Before XHP namespace support (in XHP-Lib v3), `:` is
allowed as part of an XHP class name, but it is *not* a namespace separator. It
is simply translated to `__` at runtime (this is called "name mangling"). For
example, `<ui:table>` would instantiate a global class named `xhp_ui__table`. In
all other contexts, XHP classes must be referenced with the `:` prefix (e.g.
`if ($obj is :ui:table) { ... }`).

</FbHistorical>

## Dynamic Content

The examples so far have only shown static content, but usually you'll need to include something that's generated at runtime; for this,
you can use Hack expressions directly within XHP with braces:

```hack no-extract
<xhp_class>{$some_expression}</xhp_class>
```

This also works for attributes:

```hack no-extract
<xhp_class attribute={$some_expression} />
```

More complicated expressions are also supported, for example:

```hack no-extract
use type Facebook\XHP\HTML\{div, i, strong};

class MyBasicUsageExampleClass {
  public function getInt(): int {
    return 4;
  }
}

function basic_usage_examples_get_string(): string {
  return "Hello";
}

function basic_usage_examples_get_float(): float {
  return 1.2;
}

<<__EntryPoint>>
async function basic_usage_examples_embed_hack(): Awaitable<void> {
  $xhp_float = <i>{basic_usage_examples_get_float()}</i>;

  $xhp =
    <div>
      {(new MyBasicUsageExampleClass())->getInt()}
      <strong>{basic_usage_examples_get_string()}</strong>
      {$xhp_float /* this embeds the <i /> element as a child of the <div /> */}
    </div>;
  echo await $xhp->toStringAsync();
}
```

## Attributes

Like HTML, XHP supports attributes on an XHP object. An XHP object can have zero or any number of attributes available to it. The XHP
class defines what attributes are available to objects of that class:

```hack no-extract
echo <input type="button" name="submit" value="OK" />;
```

Here the `input` class has the attributes `type`, `name` and `value`.

Some attributes are required, and XHP will throw an exception when an XHP object
is rendered (`toStringAsync()` is called) with any required attributes missing.
With `check_xhp_attribute=true` (available since HHVM 4.8) this is also a
typechecker error.

Use the [`->:` operator](/hack/expressions-and-operators/XHP-attribute-selection) to select an attribute.

## HTML Character References

In order to encode a reserved HTML character or a character that is not readily available to you, you can use HTML character references in XHP:

```hack no-extract
<?hh
echo <span>&hearts; &#9829; &#x2665;</span>;
```

The above uses HTML character reference encoding to print out the heart symbol using the explicit name, decimal notation, and hex notation.
