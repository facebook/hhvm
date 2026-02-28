
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Ensure that an invariant is satisfied




``` Hack
namespace HH;

function invariant(
  mixed $condition,
  FormatString<\PlainSprintf> $format_str,
  ...$f_args,
): void;
```




If it fails, it calls
` invariant_violation `




This function provides a way to have a variable type checked as a more
specific type than it is currently declared. A source transformation in the
runtime modifies code that looks like:




```
invariant(<condition>, 'sprintf format: %s %d', 'string', ...);
```

... is transformed to be:




```
  if (!(<condition>)) { // an Exception is thrown
    invariant_violation('sprintf format: %s', 'string', ...);
  }
  // <condition> is known to be true in the code below
```




See also:

+ [` invariant ` guide](</hack/types/type-refinement/>)




## Parameters




* ` mixed $condition `
* ` FormatString<\PlainSprintf> $format_str ` - The string that will be displayed when your
  invariant fails, with possible placeholders.
* ` ...$f_args `




## Returns




- ` void `
<!-- HHAPIDOC -->
