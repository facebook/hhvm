<?hh

<<__NativeData("XMLReader")>>
class XMLReader {

  function __construct(): void {}

  <<__Native>>
  function open(string $uri, ?string $encoding = null, int $options = 0): mixed;

  <<__Native>>
  function XML(string $source,
               ?string $encoding = null,
               int $options = 0): bool;

  <<__Native>>
  function close(): bool;

  <<__Native>>
  function read(): bool;

  <<__Native>>
  function next(?string $localname = null): bool;

  <<__Native>>
  function readString(): string;

  <<__Native>>
  function readInnerXML(): string;

  <<__Native>>
  function readOuterXML(): string;

  <<__Native>>
  function moveToNextAttribute(): bool;

  <<__Native>>
  function getAttribute(string $name): mixed;

  <<__Native>>
  function getAttributeNo(int $index): mixed;

  <<__Native>>
  function getAttributeNs(string $name, string $namespaceURI): mixed;

  <<__Native>>
  function moveToAttribute(string $name): bool;

  <<__Native>>
  function moveToAttributeNo(int $index): bool;

  <<__Native>>
  function moveToAttributeNs(string $name, string $namespaceURI): bool;

  <<__Native>>
  function moveToElement(): bool;

  <<__Native>>
  function moveToFirstAttribute(): bool;

  <<__Native>>
  function isValid(): bool;

  <<__Native>>
  function __get(mixed $name): mixed;

  <<__Native>>
  function getParserProperty(int $property): bool;

  <<__Native>>
  function lookupNamespace(string $prefix): mixed;

  <<__Native>>
  function setSchema(string $source): bool;

  <<__Native>>
  function setParserProperty(int $property, bool $value): bool;

  <<__Native>>
  function setRelaxNGSchema(string $filename): mixed;

  <<__Native>>
  function setRelaxNGSchemaSource(string $source): bool;

  <<__Native>>
  function expand(?DOMNode $basenode = null): mixed;
}
