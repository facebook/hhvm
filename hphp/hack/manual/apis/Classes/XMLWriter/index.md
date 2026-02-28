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




+ [` ->__construct() `](/apis/Classes/XMLWriter/__construct/)
+ [` ->endAttribute(): bool `](/apis/Classes/XMLWriter/endAttribute/)\
  Ends the current attribute
+ [` ->endCData(): bool `](/apis/Classes/XMLWriter/endCData/)\
  Ends the current CDATA section
+ [` ->endComment(): bool `](/apis/Classes/XMLWriter/endComment/)\
  Ends the current comment
+ [` ->endDTD(): bool `](/apis/Classes/XMLWriter/endDTD/)\
  Ends the DTD of the document
+ [` ->endDTDAttlist(): bool `](/apis/Classes/XMLWriter/endDTDAttlist/)\
  Ends the current DTD attribute list
+ [` ->endDTDElement(): bool `](/apis/Classes/XMLWriter/endDTDElement/)\
  Ends the current DTD element
+ [` ->endDTDEntity(): bool `](/apis/Classes/XMLWriter/endDTDEntity/)\
  Ends the current DTD entity
+ [` ->endDocument(): bool `](/apis/Classes/XMLWriter/endDocument/)\
  Ends the current document
+ [` ->endElement(): bool `](/apis/Classes/XMLWriter/endElement/)\
  Ends the current element
+ [` ->endPI(): bool `](/apis/Classes/XMLWriter/endPI/)\
  Ends the current processing instruction
+ [` ->flush(bool $empty = true): mixed `](/apis/Classes/XMLWriter/flush/)\
  Flushes the current buffer
+ [` ->fullEndElement(): bool `](/apis/Classes/XMLWriter/fullEndElement/)\
  End the current xml element
+ [` ->openMemory(): bool `](/apis/Classes/XMLWriter/openMemory/)\
  Create new xmlwriter using memory for string output
+ [` ->openURI(string $uri): bool `](/apis/Classes/XMLWriter/openURI/)\
  Creates a new XMLWriter using uri for the output
+ [` ->outputMemory(bool $flush = true): string `](/apis/Classes/XMLWriter/outputMemory/)\
  Returns the current buffer
+ [` ->setIndent(bool $indent): bool `](/apis/Classes/XMLWriter/setIndent/)\
  Toggles indentation on or off
+ [` ->setIndentString(string $indentstring): bool `](/apis/Classes/XMLWriter/setIndentString/)\
  Sets the string which will be used to indent each element/attribute of the
  resulting xml
+ [` ->startAttribute(string $name): bool `](/apis/Classes/XMLWriter/startAttribute/)\
  Starts an attribute
+ [` ->startAttributeNS(string $prefix, string $name, string $uri): bool `](/apis/Classes/XMLWriter/startAttributeNS/)\
  Starts a namespaced attribute
+ [` ->startCData(): bool `](/apis/Classes/XMLWriter/startCData/)\
  Starts a CDATA
+ [` ->startComment(): bool `](/apis/Classes/XMLWriter/startComment/)\
  Starts a comment
+ [` ->startDTD(string $qualifiedname, string $publicid = NULL, string $systemid = NULL): bool `](/apis/Classes/XMLWriter/startDTD/)\
  Starts a DTD
+ [` ->startDTDAttlist(string $name): bool `](/apis/Classes/XMLWriter/startDTDAttlist/)\
  Starts a DTD attribute list
+ [` ->startDTDElement(string $qualifiedname): bool `](/apis/Classes/XMLWriter/startDTDElement/)\
  Starts a DTD element
+ [` ->startDTDEntity(string $name, bool $isparam): bool `](/apis/Classes/XMLWriter/startDTDEntity/)\
  Starts a DTD entity
+ [` ->startDocument(string $version = '1.0', string $encoding = NULL, string $standalone = NULL): bool `](/apis/Classes/XMLWriter/startDocument/)\
  Starts a document
+ [` ->startElement(string $name): bool `](/apis/Classes/XMLWriter/startElement/)\
  Starts an element
+ [` ->startElementNS(mixed $prefix, string $name, string $uri): bool `](/apis/Classes/XMLWriter/startElementNS/)\
  Starts a namespaced element
+ [` ->startPI(string $target): bool `](/apis/Classes/XMLWriter/startPI/)\
  Starts a processing instruction tag
+ [` ->text(string $content): bool `](/apis/Classes/XMLWriter/text/)\
  Writes a text
+ [` ->writeAttribute(string $name, string $value): bool `](/apis/Classes/XMLWriter/writeAttribute/)\
  Writes a full attribute
+ [` ->writeAttributeNS(string $prefix, string $name, string $uri, string $content): bool `](/apis/Classes/XMLWriter/writeAttributeNS/)\
  Writes a full namespaced attribute
+ [` ->writeCData(string $content): bool `](/apis/Classes/XMLWriter/writeCData/)\
  Writes a full CDATA
+ [` ->writeComment(string $content): bool `](/apis/Classes/XMLWriter/writeComment/)\
  Writes a full comment
+ [` ->writeDTD(string $name, string $publicid = NULL, string $systemid = NULL, string $subset = NULL): bool `](/apis/Classes/XMLWriter/writeDTD/)\
  Writes a full DTD
+ [` ->writeDTDAttlist(string $name, string $content): bool `](/apis/Classes/XMLWriter/writeDTDAttlist/)\
  Writes a DTD attribute list
+ [` ->writeDTDElement(string $name, string $content): bool `](/apis/Classes/XMLWriter/writeDTDElement/)\
  Writes a full DTD element
+ [` ->writeDTDEntity(string $name, string $content, bool $pe = false, string $publicid = '', string $systemid = '', string $ndataid = ''): bool `](/apis/Classes/XMLWriter/writeDTDEntity/)\
  Writes a full DTD entity
+ [` ->writeElement(string $name, string $content = NULL): bool `](/apis/Classes/XMLWriter/writeElement/)\
  Writes a full element tag
+ [` ->writeElementNS(string $prefix, string $name, string $uri, string $content = NULL): bool `](/apis/Classes/XMLWriter/writeElementNS/)\
  Writes a full namespaced element tag
+ [` ->writePI(string $target, string $content): bool `](/apis/Classes/XMLWriter/writePI/)\
  Writes a processing instruction
+ [` ->writeRaw(string $content): bool `](/apis/Classes/XMLWriter/writeRaw/)\
  Writes a raw xml text
<!-- HHAPIDOC -->
