---
title: DOMNameSpaceNode
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
class DOMNameSpaceNode extends DOMNode {...}
```




### Public Methods ([` DOMNode `](/docs/apis/Classes/DOMNode/))




+ [` ->C14N(bool $exclusive = false, bool $with_comments = false, ?darray $xpath = NULL, ?varray $ns_prefixes = NULL): string `](/docs/apis/Classes/DOMNode/C14N/)
+ [` ->C14NFile(string $uri, bool $exclusive = false, bool $with_comments = false, ?darray $xpath = NULL, ?varray $ns_prefixes = NULL): int `](/docs/apis/Classes/DOMNode/C14Nfile/)
+ [` ->C14Nfile(string $uri, bool $exclusive = false, bool $with_comments = false, mixed $xpath = NULL, mixed $ns_prefixes = NULL): mixed `](/docs/apis/Classes/DOMNode/C14Nfile/)
+ [` ->__construct(): void `](/docs/apis/Classes/DOMNode/__construct/)
+ [` ->__debugInfo(): darray<string, mixed> `](/docs/apis/Classes/DOMNode/__debugInfo/)
+ [` ->appendChild<T as DOMNode>(DOMNode $newnode): T `](/docs/apis/Classes/DOMNode/appendChild/)\
  This functions appends a child to an existing list of children or creates
  a new list of children
+ [` ->cloneNode(bool $deep = false): this `](/docs/apis/Classes/DOMNode/cloneNode/)\
  Creates a copy of the node
+ [` ->getLineNo(): int `](/docs/apis/Classes/DOMNode/getLineNo/)\
  Gets line number for where the node is defined
+ [` ->getNodePath(): mixed `](/docs/apis/Classes/DOMNode/getNodePath/)
+ [` ->hasAttributes(): bool `](/docs/apis/Classes/DOMNode/hasAttributes/)\
  This method checks if the node has attributes
+ [` ->hasChildNodes(): bool `](/docs/apis/Classes/DOMNode/hasChildNodes/)\
  This function checks if the node has children
+ [` ->insertBefore<T as DOMNode>(DOMNode $newnode, DOMNode $refnode = NULL): T `](/docs/apis/Classes/DOMNode/insertBefore/)\
  This function inserts a new node right before the reference node
+ [` ->isDefaultNamespace(string $namespaceuri): bool `](/docs/apis/Classes/DOMNode/isDefaultNamespace/)\
  Tells whether namespaceURI is the default namespace
+ [` ->isSameNode(DOMNode $node): bool `](/docs/apis/Classes/DOMNode/isSameNode/)\
  This function indicates if two nodes are the same node
+ [` ->isSupported(string $feature, string $version): bool `](/docs/apis/Classes/DOMNode/isSupported/)\
  Checks if the asked feature is supported for the specified version
+ [` ->lookupNamespaceUri(mixed $namespaceuri): string `](/docs/apis/Classes/DOMNode/lookupNamespaceUri/)\
  Gets the namespace URI of the node based on the prefix
+ [` ->lookupPrefix(string $namespaceURI): string `](/docs/apis/Classes/DOMNode/lookupPrefix/)\
  Gets the namespace prefix of the node based on the namespace URI
+ [` ->normalize(): void `](/docs/apis/Classes/DOMNode/normalize/)\
  Normalizes the node
+ [` ->removeChild(DOMNode $node): DOMNode `](/docs/apis/Classes/DOMNode/removeChild/)\
  This functions removes a child from a list of children
+ [` ->replaceChild<T as DOMNode>(DOMNode $newchildobj, DOMNode $oldchildobj): T `](/docs/apis/Classes/DOMNode/replaceChild/)\
  This function replaces the child oldnode with the passed new node
<!-- HHAPIDOC -->
