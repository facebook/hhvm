# Extending

XHP comes with classes for all standard HTML tags, but since these are first-class objects, you can create your own XHP classes for rendering
items that are not in the standard HTML specification.

## Basics

XHP class names must follow the same rules as any other Hack class names:
Letters, numbers and `_` are allowed and the name mustn't start with a number.

<FbHistorical>

Before XHP namespace support (in XHP-Lib v3), XHP class
names could also contain `:` (now a namespace separator) and `-` (now disallowed
completely). These were translated to `__` and `_` respectively at runtime (this
is called "name mangling"). For example, `<ui:big-table>` would instantiate a
global class named `xhp_ui__big_table`.

</FbHistorical>

A custom XHP class needs to do three things:
* use the keywords `xhp class` instead of `class`
* extend `x\element` (`\Facebook\XHP\Core\element`) or, rarely, another
  [base class](/hack/XHP/interfaces)
* implement the method `renderAsync` to return an XHP object (`x\node`) or the
  respective method of the chosen base class

```basic.inc.hack
use namespace Facebook\XHP\Core as x;
use type Facebook\XHP\HTML\strong;

final xhp class introduction extends x\element {
  protected async function renderAsync(): Awaitable<x\node> {
    return <strong>Hello!</strong>;
  }
}

final xhp class intro_plain_str extends x\primitive {
  protected async function stringifyAsync(): Awaitable<string> {
    return 'Hello!';
  }
}
```

```basic.hack
<<__EntryPoint>>
async function extending_examples_basic_run(): Awaitable<void> {
  $xhp = <introduction />;
  echo await $xhp->toStringAsync()."\n";

  $xhp = <intro_plain_str />;
  echo await $xhp->toStringAsync()."\n";
}
```

<FbHistorical>

Before XHP namespace support (in XHP-Lib v3), use
`class :intro_plain_str` instead of `xhp class intro_plain_str` (no `xhp`
keyword, but requires a `:` prefix in the class name).

</FbHistorical>

## Attributes

### Syntax

Your custom class may have attributes in a similar form to XML attributes, using the `attribute` keyword:

```hack no-extract
attribute <type> <name> [= default value|@required];
```

Additionally, multiple declarations can be combined:

```hack no-extract
attribute
  int foo,
  string bar @required;
```

### Types

XHP attributes support the following types:
* `bool`, `int`, `float`, `string`, `array`, `mixed` (with **no coercion**; an `int` is not coerced into `float`, for example. You will get
an `XHPInvalidAttributeException` if you try this).
* Hack enums
* XHP-specific enums inline with the attribute in the form of `enum {item, item...}`. All values must be scalar, so they can be converted to
strings. These enums are *not* Hack enums.
* Classes or interfaces
* Generic types, with type arguments

The typechecker will raise errors if attributes are incorrect when instantiating an element (e.g., `<a href={true} />`; because XHP allows
attributes to be set in other ways (e.g., `setAttribute`), not all problems can be caught by the typechecker, and an `XHPInvalidAttributeException`
will be thrown at runtime instead in those cases.

The `->:` operator can be used to retrieve the value of an attribute.

### Required Attributes

You can specify an attribute as required with the `@required` declaration after the attribute name. If you try to render the XHP object and
have not set the required attribute, then an `XHPAttributeRequiredException` will be thrown.

```required-attributes.inc.hack
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

```required-attributes.hack
use namespace Facebook\XHP;

<<__EntryPoint>>
async function extending_examples_attributes_run(): Awaitable<void> {
  /* HH_FIXME[4314] Missing required attribute is also a typechecker error. */
  $uinfo = <user_info />;
  // Can't render :user-info for an echo without setting the required userid
  // attribute
  try {
    echo await $uinfo->toStringAsync();
  } catch (XHP\AttributeRequiredException $ex) {
    \var_dump($ex->getMessage());
  }

  /* HH_FIXME[4314] This is a typechecker error but not a runtime error. */
  $uinfo = <user_info />;
  $uinfo->setAttribute('userid', 1000);
  $uinfo->setAttribute('name', 'Joel');
  echo await $uinfo->toStringAsync();
}
```

### Nullable Attributes

For historical reasons, nullable types are inferred instead of explicitly stated. An attribute is nullable if it is not `@required` and
does not have a default value. For example:

```hack no-extract
attribute
  string iAmNotNullable @required,
  string iAmNotNullableEither = "foo",
  string butIAmNullable;
```

### Inheritance

An XHP class can inherit all attributes of another XHP class using the
following syntax:

```hack no-extract
// inherit all attributes from the <div> HTML element
attribute :Facebook:XHP:HTML:div;
```

This is most useful for XHP elements that wrap another XHP element, usually to
extend its functionality. In such cases, it should be combined with *attribute transfer*.

### Attribute Transfer

Let's say you have a class that wants to inherit attributes from `<div>`. You could do something like this:

```bad-attribute-transfer.inc.hack
use namespace Facebook\XHP\Core as x;
use type Facebook\XHP\HTML\div;

final xhp class ui_my_box extends x\element {
  attribute :Facebook:XHP:HTML:div; // inherit attributes from <div>

  protected async function renderAsync(): Awaitable<x\node> {
    // returning this will ignore any attribute set on this custom object
    // They are not transferred automatically when returning the <div>
    return <div class="my-box">{$this->getChildren()}</div>;
  }
}
```

```bad-attribute-transfer.hack
<<__EntryPoint>>
async function extending_examples_bad_attribute_transfer_run(
): Awaitable<void> {
  $my_box = <ui_my_box title="My box" />;
  // This will only bring <div class="my-box"></div> ... title= will be
  // ignored.
  echo await $my_box->toStringAsync();
}
```

`attribute :Facebook:XHP:HTML:div` causes your class to inherit all `<div>` attributes,
however, any attribute set on `<ui_my_good_box>` will be lost because the `<div>` returned from `render` will not automatically
get those attributes.

This can be addressed by using the `...` operator.

```attribute-transfer.inc.hack
use namespace Facebook\XHP\Core as x;
use type Facebook\XHP\HTML\{div, XHPAttributeClobbering_DEPRECATED};

final xhp class ui_my_good_box extends x\element {
  attribute :Facebook:XHP:HTML:div; // inherit attributes from <div>
  attribute int extra_attr;

  protected async function renderAsync(): Awaitable<x\node> {
    // returning this will transfer any attribute set on this custom object
    return <div id="id1" {...$this} class="class1">{$this->getChildren()}</div>;
  }
}
```

```attribute-transfer.hack
<<__EntryPoint>>
async function extending_examples_good_attribute_transfer_run(
): Awaitable<void> {
  $my_box =
    <ui_my_good_box
      id="id2"
      class="class2"
      extra_attr={42}
    />;
  echo await $my_box->toStringAsync();
}
```

Now, when `<ui_my_good_box>` is rendered, each `<div>` attribute will be transferred over.

Observe that `extra_attr`, which doesn't exist on `<div>`, is not transferred.
Also note that the position of `{...$this}` matters - it overrides any
duplicate attributes specified earlier, but attributes specified later override
it.

## Children

You can declare the types that your custom class is allowed to have as children
by using the `Facebook\XHP\ChildValidation\Validation` trait and implementing the
`getChildrenDeclaration()` method.

<FbHistorical>

Before XHP namespace support (in XHP-Lib v3), a special
`children` keyword with a regex-like syntax could be used instead
([examples](https://github.com/hhvm/xhp-lib/blob/v3.x/tests/ChildRuleTest.php)).
However, XHP-Lib v3 also supports `Facebook\XHP\ChildValidation\Validation`, and
it is therefore recommended to use it everywhere.

</FbHistorical>

If you don't use the child validation trait, then your class can have any
children. However, child validation still applies to any XHP objects returned
by your `renderAsync()` method that use the trait.

If an element is rendered (`toStringAsync()` is called) with children that don't
satisfy the rules in its `getChildrenDeclaration()`, an `InvalidChildrenException`
is thrown. Note that child validation only happens during rendering, no
exception is thrown before that, e.g. when the invalid child is added.

```children.inc.hack
// Conventionally aliased to XHPChild, which makes the children declarations
// easier to read (more fluid).
use namespace Facebook\XHP\{ChildValidation as XHPChild, Core as x};
use type Facebook\XHP\HTML\{body, head, html, li, ul};

xhp class my_br extends x\primitive {
  use XHPChild\Validation;

  protected static function getChildrenDeclaration(): XHPChild\Constraint {
    return XHPChild\empty();
  }

  protected async function stringifyAsync(): Awaitable<string> {
    return "\n";
  }
}

xhp class my_ul extends x\element {
  use XHPChild\Validation;

  protected static function getChildrenDeclaration(): XHPChild\Constraint {
    return XHPChild\at_least_one_of(XHPChild\of_type<li>());
  }

  protected async function renderAsync(): Awaitable<x\node> {
    return <ul>{$this->getChildren()}</ul>;
  }
}

xhp class my_html extends x\element {
  use XHPChild\Validation;

  protected static function getChildrenDeclaration(): XHPChild\Constraint {
    return XHPChild\sequence(
      XHPChild\of_type<head>(),
      XHPChild\of_type<body>(),
    );
  }

  protected async function renderAsync(): Awaitable<x\node> {
    return <html>{$this->getChildren()}</html>;
  }
}
```

```children.hack
use namespace Facebook\XHP;
use type Facebook\XHP\HTML\{body, head, li, ul};

<<__EntryPoint>>
async function extending_examples_children_run(): Awaitable<void> {
  $my_br = <my_br />;
  // Even though my-br does not take any children, you can still call the
  // appendChild method with no consequences. The consequence will come when
  // you try to render the object by something like an echo.
  $my_br->appendChild(<ul />);
  try {
    echo await $my_br->toStringAsync()."\n";
  } catch (XHP\InvalidChildrenException $ex) {
    \var_dump($ex->getMessage());
  }
  $my_ul = <my_ul />;
  $my_ul->appendChild(<li />);
  $my_ul->appendChild(<li />);
  echo await $my_ul->toStringAsync()."\n";
  $my_html = <my_html />;
  $my_html->appendChild(<head />);
  $my_html->appendChild(<body />);
  echo await $my_html->toStringAsync()."\n";
}
```

### Interfaces (categories)

XHP classes are encouraged to implement one or more interfaces (usually empty),
conventionally called "categories". Some common ones taken from the HTML
specification are declared in the `Facebook\XHP\HTML\Category` namespace.

Using such interfaces makes it possible to implement `getChildrenDeclaration()`
in other elements without having to manually list all possible child types, some
of which may not even exist yet.

```categories.inc.hack
use namespace Facebook\XHP\{
  ChildValidation as XHPChild,
  Core as x,
  HTML\Category,
};

xhp class my_text extends x\element implements Category\Phrase {
  use XHPChild\Validation;

  protected static function getChildrenDeclaration(): XHPChild\Constraint {
    return XHPChild\any_of(
      XHPChild\pcdata(),
      XHPChild\of_type<Category\Phrase>(),
    );
  }

  protected async function renderAsync(): Awaitable<x\node> {
    return <x:frag>{$this->getChildrenOfType<Category\Phrase>()}</x:frag>;
  }
}
```

```categories.hack
use type Facebook\XHP\HTML\em;

<<__EntryPoint>>
async function extending_examples_categories_run(): Awaitable<void> {
  $my_text = <my_text />;
  $my_text->appendChild(<em>"Hello!"</em>); // This is a Category\Phrase
  echo await $my_text->toStringAsync();

  $my_text = <my_text />;
  $my_text->appendChild("Bye!"); // This is pcdata, not a phrase
  // Won't print out "Bye!" because render is only returning Phrase children
  echo await $my_text->toStringAsync();
}
```

<FbHistorical>

Before XHP namespace support (in XHP-Lib v3), a special
`category` keyword could be used instead of an interface
(`category %name1, %name2;`).

</FbHistorical>

## Async

XHP and [async](/hack/asynchronous-operations/introduction) co-exist well together.
As you may have noticed, all rendering methods (`renderAsync`, `stringifyAsync`)
are declared to return an `Awaitable` and can therefore be implemented as async
functions and use `await`.

```xhp-async.inc.hack
use namespace Facebook\XHP\Core as x;

final xhp class ui_get_status extends x\element {

  protected async function renderAsync(): Awaitable<x\node> {
    $ch = \curl_init('https://metastatus.com/graph-api');
    \curl_setopt($ch, \CURLOPT_USERAGENT, 'hhvm/user-documentation example');
    $status = await \HH\Asio\curl_exec($ch);
    return <x:frag>Status is: {$status}</x:frag>;
  }
}
```

```xhp-async.hack
<<__EntryPoint>>
async function extending_examples_async_run(): Awaitable<void> {
  $status = <ui_get_status />;
  $html = await $status->toStringAsync();
  // This can be long, so just show a bit for illustrative purposes
  $sub_status = \substr($html, 0, 100);
  \var_dump($sub_status);
}
```


<FbHistorical>

In XHP-Lib v3, most rendering methods are not async, and
therefore a special `\XHPAsync` trait must be used in XHP classes that need to
`await` something during rendering.

</FbHistorical>

## HTML Helpers

The `Facebook\XHP\HTML\XHPHTMLHelpers` trait implements two behaviors:
* Giving each object a unique `id` attribute.
* Managing the `class` attribute.

<FbHistorical>

In XHP-Lib v3, this trait is called `\XHPHelpers`.

</FbHistorical>

### Unique IDs

`XHPHTMLHelpers` has a method `getID` that you can call to give your rendered custom XHP object a unique ID that can be referred to in other
parts of your code or UI framework (e.g., CSS).

```get-id.inc.hack
use namespace Facebook\XHP\Core as x;
use type Facebook\XHP\HTML\{span, XHPHTMLHelpers};

xhp class my_id extends x\element {
  attribute string id;
  use XHPHTMLHelpers;
  protected async function renderAsync(): Awaitable<x\node> {
    return <span id={$this->getID()}>This has a random id</span>;
  }
}
```

```get-id.hack
<<__EntryPoint>>
async function extending_examples_get_id_run(): Awaitable<void> {
  // This will print something like:
  // <span id="8b95a23bc0">This has a random id</span>
  $xhp = <my_id />;
  echo await $xhp->toStringAsync();
}
```

### Class Attribute Management

`XHPHTMLHelpers` has two methods to add a class name to the `class` attribute of
an XHP object. `addClass` takes a `string` and appends that `string` to the
`class` attribute (space-separated); `conditionClass` takes a `bool` and a `string`, and only
appends that `string` to the `class` attribute if the `bool` is `true`.

This is best illustrated with a standard HTML element, all of which have a
`class` attribute and use the `XHPHTMLHelpers` trait, but it works with any
XHP class, as long as it uses the trait and declares the `class` attribute
directly or through inheritance.

```add-class.hack
use type Facebook\XHP\HTML\h1;

function get_header(string $section_name): h1 {
  return (<h1 class="initial-cls">{$section_name}</h1>)
    ->addClass('added-cls')
    ->conditionClass($section_name === 'Home', 'home-cls');
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $xhp = get_header('Home');
  echo await $xhp->toStringAsync()."\n";

  $xhp = get_header('Contact');
  echo await $xhp->toStringAsync()."\n";
}
```
