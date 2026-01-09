---
title: XMLWriter
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Represents a writer that provides a non-cached, forward-only means of
generating streams or files containing XML data




## Interface Synopsis




``` Hack
class XMLWriter {...}
```




### Public Methods




+ [` ->__construct() `](/docs/apis/Classes/XMLWriter/__construct/)
+ [` ->endAttribute(): bool `](/docs/apis/Classes/XMLWriter/endAttribute/)\
  Ends the current attribute
+ [` ->endCData(): bool `](/docs/apis/Classes/XMLWriter/endCData/)\
  Ends the current CDATA section
+ [` ->endComment(): bool `](/docs/apis/Classes/XMLWriter/endComment/)\
  Ends the current comment
+ [` ->endDTD(): bool `](/docs/apis/Classes/XMLWriter/endDTD/)\
  Ends the DTD of the document
+ [` ->endDTDAttlist(): bool `](/docs/apis/Classes/XMLWriter/endDTDAttlist/)\
  Ends the current DTD attribute list
+ [` ->endDTDElement(): bool `](/docs/apis/Classes/XMLWriter/endDTDElement/)\
  Ends the current DTD element
+ [` ->endDTDEntity(): bool `](/docs/apis/Classes/XMLWriter/endDTDEntity/)\
  Ends the current DTD entity
+ [` ->endDocument(): bool `](/docs/apis/Classes/XMLWriter/endDocument/)\
  Ends the current document
+ [` ->endElement(): bool `](/docs/apis/Classes/XMLWriter/endElement/)\
  Ends the current element
+ [` ->endPI(): bool `](/docs/apis/Classes/XMLWriter/endPI/)\
  Ends the current processing instruction
+ [` ->flush(bool $empty = true): mixed `](/docs/apis/Classes/XMLWriter/flush/)\
  Flushes the current buffer
+ [` ->fullEndElement(): bool `](/docs/apis/Classes/XMLWriter/fullEndElement/)\
  End the current xml element
+ [` ->openMemory(): bool `](/docs/apis/Classes/XMLWriter/openMemory/)\
  Create new xmlwriter using memory for string output
+ [` ->openURI(string $uri): bool `](/docs/apis/Classes/XMLWriter/openURI/)\
  Creates a new XMLWriter using uri for the output
+ [` ->outputMemory(bool $flush = true): string `](/docs/apis/Classes/XMLWriter/outputMemory/)\
  Returns the current buffer
+ [` ->setIndent(bool $indent): bool `](/docs/apis/Classes/XMLWriter/setIndent/)\
  Toggles indentation on or off
+ [` ->setIndentString(string $indentstring): bool `](/docs/apis/Classes/XMLWriter/setIndentString/)\
  Sets the string which will be used to indent each element/attribute of the
  resulting xml
+ [` ->startAttribute(string $name): bool `](/docs/apis/Classes/XMLWriter/startAttribute/)\
  Starts an attribute
+ [` ->startAttributeNS(string $prefix, string $name, string $uri): bool `](/docs/apis/Classes/XMLWriter/startAttributeNS/)\
  Starts a namespaced attribute
+ [` ->startCData(): bool `](/docs/apis/Classes/XMLWriter/startCData/)\
  Starts a CDATA
+ [` ->startComment(): bool `](/docs/apis/Classes/XMLWriter/startComment/)\
  Starts a comment
+ [` ->startDTD(string $qualifiedname, string $publicid = NULL, string $systemid = NULL): bool `](/docs/apis/Classes/XMLWriter/startDTD/)\
  Starts a DTD
+ [` ->startDTDAttlist(string $name): bool `](/docs/apis/Classes/XMLWriter/startDTDAttlist/)\
  Starts a DTD attribute list
+ [` ->startDTDElement(string $qualifiedname): bool `](/docs/apis/Classes/XMLWriter/startDTDElement/)\
  Starts a DTD element
+ [` ->startDTDEntity(string $name, bool $isparam): bool `](/docs/apis/Classes/XMLWriter/startDTDEntity/)\
  Starts a DTD entity
+ [` ->startDocument(string $version = '1.0', string $encoding = NULL, string $standalone = NULL): bool `](/docs/apis/Classes/XMLWriter/startDocument/)\
  Starts a document
+ [` ->startElement(string $name): bool `](/docs/apis/Classes/XMLWriter/startElement/)\
  Starts an element
+ [` ->startElementNS(mixed $prefix, string $name, string $uri): bool `](/docs/apis/Classes/XMLWriter/startElementNS/)\
  Starts a namespaced element
+ [` ->startPI(string $target): bool `](/docs/apis/Classes/XMLWriter/startPI/)\
  Starts a processing instruction tag
+ [` ->text(string $content): bool `](/docs/apis/Classes/XMLWriter/text/)\
  Writes a text
+ [` ->writeAttribute(string $name, string $value): bool `](/docs/apis/Classes/XMLWriter/writeAttribute/)\
  Writes a full attribute
+ [` ->writeAttributeNS(string $prefix, string $name, string $uri, string $content): bool `](/docs/apis/Classes/XMLWriter/writeAttributeNS/)\
  Writes a full namespaced attribute
+ [` ->writeCData(string $content): bool `](/docs/apis/Classes/XMLWriter/writeCData/)\
  Writes a full CDATA
+ [` ->writeComment(string $content): bool `](/docs/apis/Classes/XMLWriter/writeComment/)\
  Writes a full comment
+ [` ->writeDTD(string $name, string $publicid = NULL, string $systemid = NULL, string $subset = NULL): bool `](/docs/apis/Classes/XMLWriter/writeDTD/)\
  Writes a full DTD
+ [` ->writeDTDAttlist(string $name, string $content): bool `](/docs/apis/Classes/XMLWriter/writeDTDAttlist/)\
  Writes a DTD attribute list
+ [` ->writeDTDElement(string $name, string $content): bool `](/docs/apis/Classes/XMLWriter/writeDTDElement/)\
  Writes a full DTD element
+ [` ->writeDTDEntity(string $name, string $content, bool $pe = false, string $publicid = '', string $systemid = '', string $ndataid = ''): bool `](/docs/apis/Classes/XMLWriter/writeDTDEntity/)\
  Writes a full DTD entity
+ [` ->writeElement(string $name, string $content = NULL): bool `](/docs/apis/Classes/XMLWriter/writeElement/)\
  Writes a full element tag
+ [` ->writeElementNS(string $prefix, string $name, string $uri, string $content = NULL): bool `](/docs/apis/Classes/XMLWriter/writeElementNS/)\
  Writes a full namespaced element tag
+ [` ->writePI(string $target, string $content): bool `](/docs/apis/Classes/XMLWriter/writePI/)\
  Writes a processing instruction
+ [` ->writeRaw(string $content): bool `](/docs/apis/Classes/XMLWriter/writeRaw/)\
  Writes a raw xml text
<!-- HHAPIDOC -->
