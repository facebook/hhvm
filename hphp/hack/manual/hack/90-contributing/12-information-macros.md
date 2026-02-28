# Information Macros

This website supports `note`, `tip`, `caution`, and `danger` macro messages.

These can be added with [admonition](https://docusaurus.io/markdown-features/admonitions) syntax:

```markdown
:::note
Your **content** goes here...
:::
```

## Note message

:::note
This change was introduced in [HHVM 4.136](https://hhvm.com/blog/2021/11/19/hhvm-4.136.html).
:::

The `noreturn` type can be upcasted to `dynamic`.

## Tip message

:::tip
To start learning Hack, read our [Getting Started](/hack/getting-started/quick-start) documentation!
:::

Hack lets you write code quickly, while also having safety features built in, like static type checking.

## Caution message

:::caution
We do not recommend using experimental features until they are formally announced and integrated.
:::

This website maintains documentation on Hack features that are in the *experimental* phase.

## Warning message
:::danger
Use this method with caution. **If your query contains *ANY* unescaped input, you are putting your database at risk.**
:::

While inherently dangerous, you can use `AsyncMysqlConnection::query` to query a MySQL database client.
