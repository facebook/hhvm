<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
class XMLReader {
  const int NONE;
  const int ELEMENT;
  const int ATTRIBUTE;
  const int TEXT;
  const int CDATA;
  const int ENTITY_REF;
  const int ENTITY;
  const int PI;
  const int COMMENT;
  const int DOC;
  const int DOC_TYPE;
  const int DOC_FRAGMENT;
  const int NOTATION;
  const int WHITESPACE;
  const int SIGNIFICANT_WHITESPACE;
  const int END_ELEMENT;
  const int END_ENTITY;
  const int XML_DECLARATION;
  const int LOADDTD;
  const int DEFAULTATTRS;
  const int VALIDATE;
  const int SUBST_ENTITIES;

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
  public function open(
    string $uri,
    ?string $encoding = null,
    int $options = 0,
  ): mixed;
  public function XML(
    string $source,
    ?string $encoding = null,
    int $options = 0,
  ): bool;
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
  public function getParserProperty(int $property): bool;
  public function lookupNamespace(string $prefix): mixed;
  public function setSchema(string $source): bool;
  public function setParserProperty(int $property, bool $value): bool;
  public function setRelaxNGSchema(string $filename): mixed;
  public function setRelaxNGSchemaSource(string $source): bool;
}
