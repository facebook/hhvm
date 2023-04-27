You can incrementally port code to use XHP.

Assume your output is currently handled by the following function, which might
be called from many places.

```hack no-extract
function render_component(string $text, Uri $uri): string {
  $uri = htmlspecialchars($uri->toString());
  $text = htmlspecialchars($text);
  return "<a href=\"$uri\">$text</a>";
}
```

## Convert Leaf Functions

You can start by simply using XHP in `render_component`:

```hack no-extract
async function render_component(string $text, Uri $uri): Awaitable<string> {
  $link = <a href={$uri->toString()}>{$text}</a>;
  return await $link->toStringAsync();
  // or HH\Asio\join if converting all callers to async is hard
}
```

You are converting `render_component` into a safer function without the need for explicit escaping, etc. But you are still passing
strings around in the end.

## Use a Class

You could make `render_component` into a class:

```hack no-extract
namespace ui;

class link extends x\element {
  attribute Uri uri @required;
  attribute string text @required;
  protected async function renderAsync(): Awaitable<x\node> {
    return
      <a href={$this->:uri->toString()}>{$this->:text}</a>;
  }
}
```

Keep a legacy `render_component` around while you are converting the old code that uses `render_component` to use the class.

```hack no-extract
async function render_component(string $text, Uri $uri): Awaitable<string> {
  return await (<ui:link uri={$uri} text={$text} />)->toStringAsync();
}
```
