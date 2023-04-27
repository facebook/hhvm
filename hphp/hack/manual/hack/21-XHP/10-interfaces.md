There are two important XHP types, the `\XHPChild` interface (HHVM built-in) and
the `\Facebook\XHP\Core\node` base class (declared in XHP-Lib). You will most
commonly encounter these in functions' return type annotations.

## `\XHPChild`

XHP presents a tree structure, and this interface defines what can be valid child nodes of the tree; it includes:

- all subclasses of `\Facebook\XHP\Core\node` and the advanced interfaces
  described below
- strings, integers, floats
- arrays of any of the above

Despite strings, integers, floats, and arrays not being objects, both the typechecker and HHVM consider them to implement this interface,
for parameter/return types and for `is` checks.

## `\Facebook\XHP\Core\node` (`x\node`)

The `\Facebook\XHP\Core\node` base class is implemented by all XHP objects, via
one of its two subclasses:

- `\Facebook\XHP\Core\element` (`x\element`): most common; subclasses implement a
  `renderAsync()` method that returns another `node`, and XHP-Lib automatically
  takes care of recursively rendering nested XHP objects
- `\Facebook\XHP\Core\primitive` (`x\primitive`): for very low-level nodes that
  need exact control of how the object is rendered to a string, or when the
  automatic handling of nested XHP objects is insufficient; subclasses implement
  a `stringifyAsync()` method that returns a `string` and must manually deal with
  any children

**Historical note:**
<span data-nosnippet fbIcon">(applies in FB WWW repository)</span>
Before XHP namespace support (in XHP-Lib v3), the names of
`node`, `element` and `primitive` are `\XHPRoot`, `:x:element` and
`:x:primitive` respectively.

The `\Facebook\XHP\Core` namespace is conventionally aliased as `x` (`use Facebook\XHP\Core as x;`), so you might encounter these classes as `x\node`,
`x\element` and `x\primitive`, which also mirrors their historical names.

## Advanced Interfaces

While XHP's safe-by-default features are usually beneficial, occasionally they need to be bypassed; the most common cases are:
 - Needing to embed the output from another template system when migrating to XHP.
 - Needing to embed HTML from another source, for example, Markdown or BBCode renderers.

XHP usually gets in the way of this by:
 - Escaping all variables, including your HTML code.
 - Enforcing child relationships - and XHP objects can not be marked as allowing HTML string children.

The `\Facebook\XHP\UnsafeRenderable` and `\Facebook\XHP\XHPAlwaysValidChild` interfaces allow bypassing these safety mechanisms.

**Historical note:**
<span data-nosnippet class="fbOnly fbIcon">(applies in FB WWW repository)</span>
Before XHP namespace support (in XHP-Lib v3), the names of
these interfaces are `\XHPUnsafeRenderable` and `\XHPAlwaysValidChild`.

### `\Facebook\XHP\UnsafeRenderable`

If you need to render raw HTML strings, wrap them in a class that implements this interface and provides a `toHTMLStringAsync()` method:

```md.xss-security-hole.inc.hack
use namespace Facebook\XHP;

/* YOU PROBABLY SHOULDN'T DO THIS
 *
 * Even with a scary (and accurate) name, it tends to be over-used.
 * See below for an alternative.
 */
class ExamplePotentialXSSSecurityHole implements XHP\UnsafeRenderable {
  public function __construct(private string $html) {
  }

  public async function toHTMLStringAsync(): Awaitable<string> {
    return $this->html;
  }
}
```
```md.xss-security-hole.hack no-auto-output
use type Facebook\XHP\HTML\div;

<<__EntryPoint>>
async function start(): Awaitable<void> {
  $xhp =
    <div class="markdown">
      {new ExamplePotentialXSSSecurityHole(
        md_render('Markdown goes here'),
      )}
    </div>;
  echo await $xhp->toStringAsync();
}
```

We do not provide an implementation of this interface as a generic implementation tends to be overused; instead, consider making more specific
implementations:

```md.markdown-wrapper.inc.hack
use namespace Facebook\XHP;

final class ExampleMarkdownXHPWrapper implements XHP\UnsafeRenderable {
  private string $html;

  public function __construct(string $markdown_source) {
    $this->html = md_render($markdown_source);
  }

  public async function toHTMLStringAsync(): Awaitable<string> {
    return $this->html;
  }
}
```
```md.markdown-wrapper.hack no-auto-output
use type Facebook\XHP\HTML\div;

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $xhp =
    <div class="markdown">
      {new ExampleMarkdownXHPWrapper('Markdown goes here')}
    </div>;
  echo await $xhp->toStringAsync();
}
```

### `\Facebook\XHP\AlwaysValidChild`

XHP's child validation can be bypassed by implementing this interface. Most classes that implement this interface are also implementations of
`UnsafeRenderable`, as the most common need is when a child is produced by another rendering or template system.

This can also be implemented by XHP objects, but this usually indicates that some class in `getChildrenDeclaration()` should be replaced with a more generic interface.
`AlwaysValidChild` is intentionally breaking part of XHP's safety, so should be used as sparingly as possible.

## Example

```all-in-one.inc.hack
use namespace Facebook\XHP;

final class XHPUnsafeExample implements XHP\UnsafeRenderable {
  public async function toHTMLStringAsync(): Awaitable<string> {
    /* HH_FIXME[2050] $_GET is not recognized by the typechecker */
    return '<script>'.$_GET['I_LOVE_XSS'].'</script>';
  }
}
```
```all-in-one.hack
use namespace Facebook\XHP\Core as x;
use type Facebook\XHP\HTML\{div, li};

<<__EntryPoint>>
function all_in_one_xhp_example_main(): void {
  $inputs = Map {
    '<div />' => <div />,
    '<x:frag />' => <x:frag />,
    '"foo"' => 'foo',
    '3' => 3,
    'true' => true,
    'null' => null,
    'new stdClass()' => new \stdClass(),
    'vec[<li />, <li />, <li />]' => vec[<li />, <li />, <li />],
    'XHPUnsafeExample' => new XHPUnsafeExample(),
  };

  $max_label_len = \max($inputs->mapWithKey(($k, $_) ==> \strlen($k)));
  print Str\repeat(' ', $max_label_len + 1)." | XHPRoot | XHPChild\n";
  print Str\repeat('-', $max_label_len + 1)."-|---------|----------\n";

  foreach ($inputs as $label => $input) {
    \printf(
      " %s | %-7s | %s\n",
      Str\pad_left($label, $max_label_len, ' '),
      $input is x\node ? 'yes' : 'no',
      $input is \XHPChild ? 'yes' : 'no',
    );
  }
}
```
