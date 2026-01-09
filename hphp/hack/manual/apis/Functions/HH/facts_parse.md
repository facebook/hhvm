
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

:::warning
**Deprecated:** Use `HH\Facts\extract()` instead.
:::




``` Hack
namespace HH;

function facts_parse(
  ?string $root,
  varray<string> $pathList,
  bool $allowHipHopSyntax,
  bool $useThreads,
): darray<string, ?darray<string, mixed>>;
```




Read a set of HH/PHP files and quickly extract the main features aka. "Facts"
about their contents. Compared to require'ing a source file this is much
faster and doesn't cause excessive memory consumption as compiled results are
not kept around.




The intended use of this data is to gather info about a large number of
source files to perform queries about a codebase as a whole. For example this
can be used to make an autoloader or perform reflection queries of the form
"find all classes in a source tree which match pattern XYZ" etc.




The useThreads argument specifies whether to use a number of worker threads
equal to the number of system CPU cores for parsing. This feature should only
be used as part of warm-up operations and not on a running system which may
have a high CPU load with new/in-flight requests.




The allowHipHopSyntax argument enables HH syntax in all files parsed. By
default only files starting with ` <?hh ` may have HH syntax.




Each pathList item will map to one result entry in the returned array. If
there were any errors when parsing a file the result entry for that file will
be null, otherwise a result will be of the form:




array(
'sha1sum' => SHA1 Hash of source text,
'types' => details of 'types' in the file (traits/classes/interfaces)
array(
array(
'name' => string name of this 'type',
'kindOf' => string =
'class'|'interface'|'enum'|'trait'|'unknown'|'mixed',
'baseTypes' => array(string) of base 'types' for this type,
'flags' => integer with combination of flags from FactParseFlags,
'requireImplements' => array(string) of interfaces users of this
trait must implement (only present when kindOf = trait).
'requireExtends' => array(string) of classes which users of this
trait or interface must extend (only present when kindOf =
trait | interface)
),
array(
'name' => string,
'baseTypes' => array(string),
),
...
),
'constants' => array(string) constants defined in this file,
'functions' => array(string) functions defined in this file,
'typeAliases' => array(string) type aliases defined in this file,
)




pathList items which are not absolute are joined with a non-null "root"
argument.




## Parameters




+ ` ?string $root `
+ ` varray<string> $pathList `
+ ` bool $allowHipHopSyntax `
+ ` bool $useThreads `




## Returns




* ` darray<string, ?darray<string, mixed>> `
<!-- HHAPIDOC -->
