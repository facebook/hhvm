<?hh // partial

<<__NativeData>>
class XMLReader {

  public function __construct(): void {}

  <<__Native>>
  public function open(string $uri, ?string $encoding = null, int $options = 0): mixed;

  <<__Native>>
  public function XML(string $source,
               ?string $encoding = null,
               int $options = 0): bool;

  <<__Native>>
  public function close(): bool;

  <<__Native>>
  public function read(): bool;

  <<__Native>>
  public function next(?string $localname = null): bool;

  <<__Native>>
  public function readString(): string;

  <<__Native>>
  public function readInnerXML(): string;

  <<__Native>>
  public function readOuterXML(): string;

  <<__Native>>
  public function moveToNextAttribute(): bool;

  <<__Native>>
  public function getAttribute(string $name): mixed;

  <<__Native>>
  public function getAttributeNo(int $index): mixed;

  <<__Native>>
  public function getAttributeNs(string $name, string $namespaceURI): mixed;

  <<__Native>>
  public function moveToAttribute(string $name): bool;

  <<__Native>>
  public function moveToAttributeNo(int $index): bool;

  <<__Native>>
  public function moveToAttributeNs(string $name, string $namespaceURI): bool;

  <<__Native>>
  public function moveToElement(): bool;

  <<__Native>>
  public function moveToFirstAttribute(): bool;

  <<__Native>>
  public function isValid(): bool;

  <<__Native>>
  public function getParserProperty(int $property): bool;

  <<__Native>>
  public function lookupNamespace(string $prefix): mixed;

  <<__Native>>
  public function setSchema(string $source): bool;

  <<__Native>>
  public function setParserProperty(int $property, bool $value): bool;

  <<__Native>>
  public function setRelaxNGSchema(string $filename): mixed;

  <<__Native>>
  public function setRelaxNGSchemaSource(string $source): bool;

  <<__Native>>
  public function expand(?DOMNode $basenode = null): mixed;
}
