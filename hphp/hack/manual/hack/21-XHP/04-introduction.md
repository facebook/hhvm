XHP provides a native XML-like representation of output (which is usually HTML). This allows UI code to be typechecked, and automatically
avoids several common issues such as cross-site scripting (XSS) and double-escaping. It also applies other validation rules, e.g., `<head>`
must contain `<title>`.

Using traditional interpolation, a simple page could look like this:

```hack no-extract
$user_name = 'Fred';
echo "<tt>Hello <strong>$user_name</strong></tt>";
```

However, with XHP, it looks like this:

```hack no-extract
$user_name = 'Fred';
$xhp = <tt>Hello <strong>{$user_name}</strong></tt>;
echo await $xhp->toStringAsync();
```

The first example uses string interpolation to output the HTML, while the second has no quotation marks&mdash;meaning that the syntax is
fully understood by Hack&mdash;but this does not mean that all you need to do is remove quotation marks. Other steps needed include:
 - Use curly braces to include variables - e.g., `"<a>$foo</a>"` becomes `<a>{$foo}</a>`.
 - As XHP is XML-like, all elements must be closed - e.g., `"<br>"` becomes `<br />`.
 - Make sure your HTML is properly nested.
 - Remove all HTML/attribute escaping - e.g., you don't need to call `htmlspecialchars` before including a variable in your XHP
output; and if you do, it will be double-escaped.

## Why use XHP?

The initial reason for most users is because it is *safe by default*: all variables are automatically escaped in a
context-appropriate way (e.g., there are different rules for escaping attribute values vs. text nodes). In addition, XHP
is understood by the typechecker, making sure that you don't pass invalid attribute values. A common example of this is `border="3"`,
but `border` is an on/off attribute, so a value of 3 doesn't make sense.

For users experienced with XHP, the biggest advantage is that it is easy to add custom 'elements' with your own behavior,
which can then be used like plain HTML elements. For example, this site defines an `<a_post>` tag that has the same interface
as a standard `<a>` tag, but makes a POST request instead of a GET request:

```hack no-extract
use namespace Facebook\XHP\Core as x;
use type Facebook\XHP\HTML\{XHPHTMLHelpers, a, form};


final xhp class a_post extends x\element {
  use XHPHTMLHelpers;

  attribute string href @required;
  attribute string target;

  <<__Override>>
  protected async function renderAsync(): Awaitable<x\node> {
    $id = $this->getID();

    $anchor = <a>{$this->getChildren()}</a>;
    $form = (
      <form
        id={$id}
        method="post"
        action={$this->:href}
        target={$this->:target}
        class="postLink">
        {$anchor}
      </form>
    );

    $anchor->setAttribute(
      'onclick',
      'document.getElementById("'.$id.'").submit(); return false;',
    );
    $anchor->setAttribute('href', '#');

    return $form;
  }
}
```

A little CSS is needed so that the `<form>` doesn't create a block element:

```
form.postLink {
  display: inline;
}
```

At this point, the new element can be used like any built-in element:

```hack no-extract
use type Facebook\XHP\HTML\a;
use type HHVM\UserDocumentation\a_post;

<<__EntryPoint>>
async function intro_examples_a_a_post(): Awaitable<void> {
  $get_link = <a href="http://www.example.com">I'm a normal link</a>;
  $post_link =
    <a_post href="http://www.example.com">I make a POST REQUEST</a_post>;

  echo await $get_link->toStringAsync();
  echo "\n";
  echo await $post_link->toStringAsync();
}
```.expectf
<a href="http://www.example.com">I'm a normal link</a>
<form id="%s" method="post" action="http://www.example.com" class="postLink"><a onclick="document.getElementById(&quot;%s&quot;).submit(); return false;" href="#">I make a POST REQUEST</a></form>
```

## Runtime Validation

Since XHP objects are first-class and not just strings, a whole slew of validation can occur to ensure that your UI does not have subtle bugs:

```hack no-extract
function intro_examples_tag_matching_validation_using_string(): void {
  echo '<div class="section-header">';
  echo '<a href="#use">You should have used <span class="xhp">XHP</naps></a>';
  echo '</div>';
}

async function intro_examples_tag_matching_validation_using_xhp(
): Awaitable<void> {
  // Typechecker error
  // Fatal syntax error at runtime
  $xhp =
    <div class="section-header">
      <a href="#use">You should have used <span class="xhp">XHP</naps></a>
    </div>;
  echo await $xhp->toStringAsync();
}

<<__EntryPoint>>
async function intro_examples_tag_matching_validation_run(): Awaitable<void> {
  intro_examples_tag_matching_validation_using_string();
  await intro_examples_tag_matching_validation_using_xhp();
}
```

The above code won't typecheck or run because the XHP validator will see that `<span>` and `<naps>` tags are mismatched; however,
the following code will typecheck correctly but fail to run, because while the tags are matched, they are not nested correctly
(according to the HTML specification), and nesting verification only happens at runtime:

```hack no-extract
use namespace Facebook\XHP;
use type Facebook\XHP\HTML\{i, ul};

function intro_examples_allowed_tag_validation_using_string(): void {
  echo '<ul><i>Item 1</i></ul>';
}

async function intro_examples_allowed_tag_validation_using_xhp(
): Awaitable<void> {
  try {
    $xhp = <ul><i>Item 1</i></ul>;
    echo await $xhp->toStringAsync();
  } catch (XHP\InvalidChildrenException $ex) {
    // We will get here because an <i> cannot be nested directly below a <ul>
    \var_dump($ex->getMessage());
  }
}
```

```hack no-extract
<<__EntryPoint>>
async function intro_examples_allowed_tag_validation_run(): Awaitable<void> {
  intro_examples_allowed_tag_validation_using_string();
  echo \PHP_EOL.\PHP_EOL;
  await intro_examples_allowed_tag_validation_using_xhp();
}
```

## Security

String-based entry and validation are prime candidates for cross-site scripting (XSS). You can get around this by using special
functions like [`htmlspecialchars`](http://php.net/manual/en/function.htmlspecialchars.php), but then you have to actually remember
to use those functions. XHP automatically escapes reserved HTML characters to HTML entities before output.

```hack no-extract
use type Facebook\XHP\HTML\{body, head, html};

function intro_examples_avoid_xss_using_string(string $could_be_bad): void {
  // Could call htmlspecialchars() here
  echo '<html><head/><body> '.$could_be_bad.'</body></html>';
}

async function intro_examples_avoid_xss_using_xhp(
  string $could_be_bad,
): Awaitable<void> {
  // The string $could_be_bad will be escaped to HTML entities like:
  // <html><head></head><body>&lt;blink&gt;Ugh&lt;/blink&gt;</body></html>
  $xhp =
    <html>
      <head />
      <body>{$could_be_bad}</body>
    </html>;
  echo await $xhp->toStringAsync();
}

async function intro_examples_avoid_xss_run(
  string $could_be_bad,
): Awaitable<void> {
  intro_examples_avoid_xss_using_string($could_be_bad);
  echo \PHP_EOL.\PHP_EOL;
  await intro_examples_avoid_xss_using_xhp($could_be_bad);
}

<<__EntryPoint>>
async function intro_examples_avoid_xss_main(): Awaitable<void> {
  await intro_examples_avoid_xss_run('<blink>Ugh</blink>');
}
```
