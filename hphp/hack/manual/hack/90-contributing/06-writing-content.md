# Writing Content

The website has three content sections:

* [Hack
  guides](https://github.com/hhvm/user-documentation/tree/main/guides/hack)
  describe the Hack language
* [HHVM
  guides](https://github.com/hhvm/user-documentation/tree/main/guides/hhvm)
  describe how to install, configure and run the HHVM runtime
* The API reference is generated from the API definitions, and this
  site [includes
  examples](https://github.com/hhvm/user-documentation/tree/main/api-examples)
  for each API

<FbInfo>
Internally, the sources for all of the Hack, HHVM, HSL and API guides are stored under the same directory, `fbcode/hphp/hack/manual/`. The Hack guides are set up to automatically mirror those on `hhvm/user-documentation`
</FbInfo>

## File naming conventions

A guide is a folder with a numeric prefix:

```
$ ls guides/hack/
01-getting-started/
02-source-code-fundamentals/
...
```

These numbers are used to control the ordering of guides on the
website, and are not shown in the UI. They do not need to be
sequential; there can be gaps.

Within each folder, there are Markdown files (written in [Markdown](https://docusaurus.io/markdown-features)).

```
$ ls guides/hack/01-getting-started/
01-quick-start.md
02-tools.md
```

Pages use the same numbering system to control their order within a
guide.

## Writing conventions

Guides are written in a relatively informal tone, addressing the
reader as "you". Assume the reader has some programming knowledge but
no familiarity with Hack.

Each page should have a clear purpose, starting with a short
description of the concept it is describing. Examples should always be
provided, preferably under 15 lines.

We assume the reader has a relatively recent version of
HHVM. References to very old features will be removed, but we provide
[Docker images of historical site
versions](/hack/contributing/introduction#running-an-old-version) if
needed (from before we migrated to Docusaurus in 2025).

It is not necessary to document all the incorrect ways a feature can be
used. Content should focus on correct usage, and users can rely on the
Hack type checker to tell them when they have done something wrong.

## Links

Internal links should be **absolute paths** without a domain,
e.g. `/hack/contributing/introduction`.

Image paths should be relative to `static`,
e.g. `/img/imagename.png`.

### Validating links

To make sure all links between pages are valid, run `yarn docusaurus build`. This will check all links in the documentation and report any broken ones. It is generally advised to run this command before submitting a diff to ensure that all links are valid and that the changes do not break upstream publishing pipelines.
