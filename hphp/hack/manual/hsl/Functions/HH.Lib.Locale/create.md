
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Create a new ` Locale ` object




``` Hack
namespace HH\Lib\Locale;

function create(
  string $locale,
): Locale;
```




The input should be of the form ` country[.encoding] `, for example:
`` "C" ``, ``` en_US ```, ```` en_US.UTF-8 ````.




If present, the encoding currently **must** be 'UTF-8'.




This will throw on 'magic' locales such as:

+ the empty string: use ` from_environment() `
+ ` '0' `: use `` get_native() ``




## Parameters




* ` string $locale `




## Returns




- ` Locale `
<!-- HHAPIDOC -->
