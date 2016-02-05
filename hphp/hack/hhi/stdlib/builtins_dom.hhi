<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

const XML_ATTRIBUTE_DECL_NODE = 16;
const XML_ATTRIBUTE_NODE = 2;
const XML_CDATA_SECTION_NODE = 4;
const XML_COMMENT_NODE = 8;
const XML_DOCUMENT_FRAG_NODE = 11;
const XML_DOCUMENT_NODE = 9;
const XML_DOCUMENT_TYPE_NODE = 10;
const XML_DTD_NODE = 14;
const XML_ELEMENT_DECL_NODE = 15;
const XML_ELEMENT_NODE = 1;
const XML_ENTITY_DECL_NODE = 17;
const XML_ENTITY_NODE = 6;
const XML_ENTITY_REF_NODE = 5;
const XML_HTML_DOCUMENT_NODE = 13;
const XML_NAMESPACE_DECL_NODE = 18;
const XML_NOTATION_NODE = 12;
const XML_PI_NODE = 7;
const XML_TEXT_NODE = 3;

const XML_LOCAL_NAMESPACE = 18;

const XML_ATTRIBUTE_CDATA = 1;
const XML_ATTRIBUTE_ENTITY = 6;
const XML_ATTRIBUTE_ENUMERATION = 9;
const XML_ATTRIBUTE_ID = 2;
const XML_ATTRIBUTE_IDREF = 3;
const XML_ATTRIBUTE_IDREFS = 4;
const XML_ATTRIBUTE_NMTOKEN = 7;
const XML_ATTRIBUTE_NMTOKENS = 8;
const XML_ATTRIBUTE_NOTATION = 10;

const int DOM_PHP_ERR = 0;
const int DOM_INDEX_SIZE_ERR = 1;
const int DOMSTRING_SIZE_ERR = 2;
const int DOM_HIERARCHY_REQUEST_ERR = 3;
const int DOM_WRONG_DOCUMENT_ERR = 4;
const int DOM_INVALID_CHARACTER_ERR = 5;
const int DOM_NO_DATA_ALLOWED_ERR = 6;
const int DOM_NO_MODIFICATION_ALLOWED_ERR = 7;
const int DOM_NOT_FOUND_ERR = 8;
const int DOM_NOT_SUPPORTED_ERR = 9;
const int DOM_INUSE_ATTRIBUTE_ERR = 10;
const int DOM_INVALID_STATE_ERR = 11;
const int DOM_SYNTAX_ERR = 12;
const int DOM_INVALID_MODIFICATION_ERR = 13;
const int DOM_NAMESPACE_ERR = 14;
const int DOM_INVALID_ACCESS_ERR = 15;
const int DOM_VALIDATION_ERR = 16;

function dom_import_simplexml(SimpleXMLElement $node): DOMElement;

class DOMAttr extends DOMNode {
  public string $name;
  public DOMElement $ownerElement;
  public bool $schemaTypeInfo;
  public bool $specified;
  public string $value;
  public function __construct($name, ?string $value = NULL );
  public function isId(): bool;
}

class DOMCdataSection extends DOMText {
  public function __construct(string $value);
}

class DOMCharacterData extends DOMNode {
  public string $data;
  public int $length;
  public function appendData(string $data): void;
  public function deleteData(int $offset, int $count): void;
  public function insertData(int $offset, string $data): void;
  public function replaceData(int $offset, int $count, string $data): void;
  public function substringData(int $offset, int $count): string;
}

class DOMComment extends DOMCharacterData {
  public function __construct(?string $value = NULL);
}


class DOMDocument extends DOMNode {
  public string $actualEncoding;
  //public DOMConfiguration $config; // Class undefined: DOMConfiguration - https://bugs.php.net/bug.php?id=55012
  public mixed $config;
  public DOMDocumentType $doctype;
  public DOMElement $documentElement;
  public string $documentURI;
  public string $encoding;
  public bool $formatOutput;
  public DOMImplementation $implementation;
  public bool $preserveWhiteSpace = true;
  public bool $recover;
  public bool $resolveExternals;
  public bool $standalone;
  public bool $strictErrorChecking = true;
  public bool $substituteEntities;
  public bool $validateOnParse = false;
  public string $version;
  public string $xmlEncoding;
  public bool $xmlStandalone;
  public string $xmlVersion;
  public function __construct(?string $version = NULL, ?string $encoding = NULL);
  public function createAttribute(string $name): DOMAttr;
  public function createAttributeNS(string $namespaceURI, string $qualifiedName): DOMAttr;
  public function createCDATASection(string $data): DOMCdataSection;
  public function createComment(string $data): DOMComment;
  public function createDocumentFragment(): DOMDocumentFragment;
  public function createElement(string $name, ?string $value = NULL): DOMElement;
  public function createElementNS(string $namespaceURI, string $qualifiedName, ?string $value = NULL): DOMElement;
  public function createEntityReference(string $name): DOMEntityReference;
  public function createProcessingInstruction(string $target, ?string $data = NULL): DOMProcessingInstruction;
  public function createTextNode(string $content): DOMText;
  public function getElementById(string $elementId): DOMElement;
  public function getElementsByTagName(string $name): DOMNodeList;
  public function getElementsByTagNameNS(string $namespaceURI, string $localName): DOMNodeList;
  public function importNode(DOMNode $importedNode, bool $deep = false): DOMNode;
  public function load(string $filename, int $options = 0 ): mixed;
  public function loadHTML(string $source, int $options = 0 ): bool;
  public function loadHTMLFile(string $filename, int $options = 0 ): bool;
  public function loadXML(string $source, int $options = 0 ): mixed;
  public function normalizeDocument(): void;
  public function registerNodeClass(string $baseclass, string $extendedclass): bool;
  public function relaxNGValidate(string $filename): bool;
  public function relaxNGValidateSource(string $source): bool;
  public function save(string $filename, int $options = 0): int;
  public function saveHTML(?DOMNode $node = NULL): string;
  public function saveHTMLFile(string $filename): int;
  public function saveXML(?DOMNode $node = NULL, int $options = 0): string;
  public function schemaValidate(string $filename, int $flags = 0): bool;
  public function schemaValidateSource(string $source, int $flags = 0): bool;
  public function validate(): bool;
  public function xinclude(int $options = 0): int;
}

class DOMDocumentFragment extends DOMNode {
  public function appendXML(string $data): bool;
}

class  DOMDocumentType extends DOMNode {
  public string $publicId;
  public string $systemId;
  public string $name;
  public DOMNamedNodeMap $entities;
  public DOMNamedNodeMap $notations;
  public string $internalSubset;
}

class  DOMElement extends DOMNode {
  public bool $schemaTypeInfo;
  public string $tagName;
  public function __construct(string $name, ?string $value = NULL , ?string $namespaceURI = NULL);
  public function getAttribute(string $name): string;
  public function getAttributeNode(string $name): DOMAttr;
  public function getAttributeNodeNS(string $namespaceURI, string $localName): DOMAttr;
  public function getAttributeNS(string $namespaceURI, string $localName): string;
  public function getElementsByTagName(string $name): DOMNodeList;
  public function getElementsByTagNameNS(string $namespaceURI, string $localName): DOMNodeList;
  public function hasAttribute(string $name): bool;
  public function hasAttributeNS(string $namespaceURI, string $localName): bool;
  public function removeAttribute(string $name): bool;
  public function removeAttributeNode(DOMAttr $oldnode): bool;
  public function removeAttributeNS(string $namespaceURI, string $localName): bool;
  public function setAttribute(string $name, string $value): DOMAttr;
  public function setAttributeNode(DOMAttr $attr): DOMAttr;
  public function setAttributeNodeNS(DOMAttr $attr): DOMAttr;
  public function setAttributeNS(string $namespaceURI, string $qualifiedName, string $value): void;
  public function setIdAttribute(string $name, bool $isId): void;
  public function setIdAttributeNode(DOMAttr $attr, bool $isId): void;
  public function setIdAttributeNS(string $namespaceURI, string $localName, bool $isId): void;
}

class  DOMEntity extends DOMNode {
  public string $publicId;
  public string $systemId;
  public string $notationName;
  public string $actualEncoding;
  public string $encoding;
  public string $version;
}

class  DOMEntityReference extends DOMNode {
  public function __construct($name);
}

class  DOMException extends Exception {
  public int $code;
}

class DOMImplementation {
  public function __construct();
  public function createDocument(?string $namespaceURI = NULL, ?string $qualifiedName = NULL, ?DOMDocumentType $doctype = NULL): DOMDocument;
  public function createDocumentType(?string $qualifiedName = NULL, ?string $publicId = NULL, ?string $systemId = NULL): DOMDocumentType;
  public function hasFeature(string $feature, string $version): bool;
}

class DOMNamedNodeMap implements Traversable<DOMNode> {
  public int $length;
  public function getNamedItem(string $name): DOMNode;
  public function getNamedItemNS(string $namespaceURI, string $localName): DOMNode;
  public function item(int $index): DOMNode;
}

class DOMNode {
  public string $nodeName;
  public string $nodeValue;
  public int $nodeType;
  public DOMNode $parentNode;
  public DOMNodeList $childNodes;
  public DOMNode $firstChild;
  public DOMNode $lastChild;
  public DOMNode $previousSibling;
  public DOMNode $nextSibling;
  public DOMNamedNodeMap $attributes;
  public DOMDocument $ownerDocument;
  public string $namespaceURI;
  public string $prefix;
  public string $localName;
  public string $baseURI;
  public string $textContent;
  public function appendChild(DOMNode $newnode): DOMNode;
  public function C14N(bool $exclusive = false, bool $with_comments = false, ?array $xpath = NULL, ?array $ns_prefixes = NULL): string;
  public function C14NFile(string $uri, bool $exclusive = false, bool $with_comments = false, ?array $xpath = NULL, ?array $ns_prefixes = NULL): int;
  public function cloneNode(bool $deep = false): DOMNode;
  public function getLineNo(): int;
  public function getNodePath(): string;
  public function hasAttributes(): bool;
  public function hasChildNodes(): bool;
  public function insertBefore(DOMNode $newnode, ?DOMNode $refnode = NULL): DOMNode;
  public function isDefaultNamespace(string $namespaceURI): bool;
  public function isSameNode(DOMNode $node): bool;
  public function isSupported(string $feature, string $version): bool;
  public function lookupNamespaceURI(string $prefix): string;
  public function lookupPrefix(string $namespaceURI): string;
  public function normalize(): void;
  public function removeChild(DOMNode $oldnode): DOMNode;
  public function replaceChild(DOMNode $newnode, DOMNode $oldnode): DOMNode;
}

class DOMNodeList implements Traversable<DOMNode> {
  public int $length;
  public function item(int $index): DOMNode;
}

class DOMNotation extends DOMNode {
  public string $publicId;
  public string $systemId;
}

class DOMProcessingInstruction extends DOMNode {
  public string $target;
  public string $data;
  public function __construct($name, ?string $value = NULL );
}

class DOMText extends DOMCharacterData {
  public string $wholeText;
  public function __construct(?string $value = NULL);
  public function isWhitespaceInElementContent(): bool;
  public function splitText(int $offset): DOMText;
}

class  DOMXPath {
  public DOMDocument $document;
  public function __construct($doc);
  public function evaluate(string $expression, ?DOMNode $contextnode = NULL, bool $registerNodeNS = true): mixed;
  public function query(string $expression, ?DOMNode $contextnode = NULL, bool $registerNodeNS = true): DOMNodeList;
  public function registerNamespace(string $prefix, string $namespaceURI): bool;
  public function registerPhpFunctions(mixed $restrict = NULL): void;
}
