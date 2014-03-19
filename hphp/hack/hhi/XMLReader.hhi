<?hh // decl
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
class XMLReader {
  const int NONE = 0;
  const int ELEMENT = 0;
  const int ATTRIBUTE = 0;
  const int TEXT = 0;
  const int CDATA = 0;
  const int ENTITY_REF = 0;
  const int ENTITY = 0;
  const int PI = 0;
  const int COMMENT = 0;
  const int DOC = 0;
  const int DOC_TYPE = 0;
  const int DOC_FRAGMENT = 0;
  const int NOTATION = 0;
  const int WHITESPACE = 0;
  const int SIGNIFICANT_WHITESPACE = 0;
  const int END_ELEMENT = 0;
  const int END_ENTITY = 0;
  const int XML_DECLARATION = 0;
  const int LOADDTD = 0;
  const int DEFAULTATTRS = 0;
  const int VALIDATE = 0;
  const int SUBST_ENTITIES = 0;

  public int $attributeCount;
  public string $baseURI;
  public int $depth;
  public bool $hasAttributes;
  public bool $hasValue;
  public bool $isDefault;
  public bool $isEmptyElement;
  public string $localName;
  public string $name;
  public string $namespaceURI;
  public int $nodeType;
  public string $prefix;
  public string $value;
  public string $xmlLang;

  public function __construct();
  public function open(string $uri, ?string $encoding = null, int $options = 0): bool;
  public function XML(string $source, ?string $encoding = null, int $options = 0): bool;
  public function close(): bool;
  public function read(): bool;
  public function next(?string $localname = null): bool;
  public function readString(): string;
  public function readInnerXML(): string;
  public function readOuterXML(): string;
  public function moveToNextAttribute(): bool;
  public function getAttribute(string $name): mixed;
  public function getAttributeNo(int $index): mixed;
  public function getAttributeNs(string $name, string $namespaceURI): mixed;
  public function moveToAttribute(string $name): bool;
  public function moveToAttributeNo(int $index): bool;
  public function moveToAttributeNs(string $name, string $namespaceURI): bool;
  public function moveToElement(): bool;
  public function moveToFirstAttribute(): bool;
  public function isValid(): bool;
  public function __get($name);
  public function getParserProperty(int $property): bool;
  public function lookupNamespace(string $prefix): mixed;
  public function setSchema(string $source): bool;
  public function setParserProperty(int $property, bool $value): bool;
  public function setRelaxNGSchema(string $filename): bool;
  public function setRelaxNGSchemaSource(string $source): bool;
}
