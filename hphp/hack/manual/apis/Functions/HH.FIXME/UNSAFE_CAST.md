
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

` UNSAFE_CAST ` allows you to lie to the type checker




``` Hack
namespace HH\FIXME;

function UNSAFE_CAST<Tin, Tout>(
  Tin $t,
  ?\HH\FormatString<nothing> $msg = NULL,
): Tout;
```




**This is almost
always a bad idea**. You might get an exception, or you might get an
unexpected value. Even scarier, you might cause an exception in a
totally different part of the codebase.




```
UNSAFE_CAST<Dog, Cat>($my_dog, 'my reason here')->meow()
```




In this example, the type checker will accept the code, but the code
will still crash when you run it (no such method "meow" on "Dog").




` UNSAFE_CAST ` has no runtime effect. It only affects the type
checker. The above example will run the same as `` $my_dog->meow() ``.




## You can fix it!




It is always possible to write code without ` UNSAFE_CAST ` and without
`` HH_FIXME ``. This usually requires changing type signatures and some
refactoring. Your code will be more reliable, the type checker can
help you, and future changes will be less scary.




` UNSAFE_CAST ` is still better than `` HH_FIXME ``, because ``` HH_FIXME ```
applies the entire next line, and ```` UNSAFE_CAST ```` applies to a single
expression.




## Parameters




+ ` Tin $t `
+ ` ?\HH\FormatString<nothing> $msg = NULL `




## Returns




* ` Tout `
<!-- HHAPIDOC -->
