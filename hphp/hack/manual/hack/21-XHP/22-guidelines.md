# Guidelines

Here are some general guidelines to know and follow when using XHP. In addition to its [basic usage](/hack/XHP/basic-usage),
[available methods](/hack/XHP/methods) and [extending XHP with your own objects](/hack/XHP/extending), these are other tips to make the best use of XHP.

## Validation of Attributes and Children

The constraints of XHP object children and attributes are done at various times:
* Children constraints are validated at render-time (when `toStringAsync` is called).
* Attributes provided directly (`<your_class attr="value">`) are validated by
  the typechecker.
* Attribute names set by `setAttribute` are only validated at render-time.

## Use Contexts to Access Higher Level Information

If you have a parent object, and you want to give information to some object further down the UI tree (e.g., `<ul>` to `<li>`), you
can set a context for those lower objects and the lower objects can retrieve them. You use `setContext` and `getContext`

```context.inc.hack
use namespace Facebook\XHP\{ChildValidation as XHPChild, Core as x};
use type Facebook\XHP\HTML\{dd, dl, dt};

final xhp class ui_myparent extends x\element {
  use XHPChild\Validation;
  attribute string text @required;

  protected static function getChildrenDeclaration(): XHPChild\Constraint {
    return XHPChild\of_type<ui_mychild>();
  }

  protected async function renderAsync(): Awaitable<x\node> {
    return (
      <dl>
        <dt>Text</dt>
        <dd>{$this->:text}</dd>
        <dt>Child</dt>
        <dd>{$this->getChildren()}</dd>
      </dl>
    );
  }
}

final xhp class ui_mychild extends x\element {
  attribute string text @required;

  protected async function renderAsync(): Awaitable<x\node> {
    if ($this->getContext('hint') === 'Yes') {
      return <x:frag>{$this->:text.'...and more'}</x:frag>;
    }
    return <x:frag>{$this->:text}</x:frag>;
  }
}

async function guidelines_examples_context_run(string $s): Awaitable<void> {
  $xhp = (
    <ui_myparent text={$s}>
      <ui_mychild text="Go" />
    </ui_myparent>
  );
  $xhp->setContext('hint', $s);
  echo await $xhp->toStringAsync();
}
```
```context.hack
<<__EntryPoint>>
async function startHere(): Awaitable<void> {
  await guidelines_examples_context_run('No');
  echo "\n\n";
  await guidelines_examples_context_run('Yes');
}
```

Context is only passed down the tree at render time; this allows you to construct a tree without having to thread through data. In
general, you should only call `getContext` in the `render` method of the child.

Common uses include:
 - current user ID
 - current theme in multi-theme sites
 - current browser/device type

## Don't Add Public Methods to Your XHP Components

XHP objects should only be interacted with by their attributes, contexts, position in the tree, and rendering them. At Facebook,
we haven't yet come across any situations where public methods are a better solution than these interfaces.

## Use Inheritance Minimally

If you need an XHP object to act like another, but slightly modified, use
composition (the would-be subclass can instead return an instance of the
original XHP class from its `renderAsync` method).

## Remember No Dollar Signs

Attribute declarations look like Hack property declarations:

```hack no-extract
public string $prop;
attribute string attr;
```

A key difference is that there is no `$` in front of the attribute name.
