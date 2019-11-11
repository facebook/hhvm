<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
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

class DOMXPath {
  public function __construct($doc);
  public function evaluate(string $expr, $context = null);
  public function query(string $expr, $context = null);
  public function registerNamespace(string $prefix, string $uri);
  public function registerPHPFunctions($funcs = null);
}

class DOMNodeList<Tnode as DOMNode> implements Traversable<Tnode> {
  /* Properties */
  /* readonly */ public int $length;

  // Methods
  public function item(int $index): Tnode;
}

class DOMNamedNodeMap<Tnode as DOMNode>
  implements KeyedTraversable<string, Tnode> {

  /* Properties */
  /* readonly */ public int $length;

  /* Methods */
  public function getNamedItem(string $name): Tnode;
  public function getNamedItemNS(
    string $namespaceURI,
    string $localName,
  ): Tnode;
  public function item(int $index): Tnode;
}

class DOMNode {
  // properties
  /* readonly */ public string $nodeName;
  public string $nodeValue;
  /* readonly */ public int $nodeType;
  /* readonly */ public $parentNode;
  /* readonly */ public DOMNodeList<DOMNode> $childNodes;
  /* readonly */ public ?DOMNode $firstChild;
  /* readonly */ public ?DOMNode $lastChild;
  /* readonly */ public ?DOMNode $previousSibling;
  /* readonly */ public ?DOMNode $nextSibling;
  /* readonly */ public ?DOMNamedNodeMap<DOMAttr> $attributes;
  /* readonly */ public DOMDocument $ownerDocument;
  /* readonly */ public ?string $namespaceURI;
  public ?string $prefix;
  /* readonly */ public string $localName;
  /* readonly */ public ?string $baseURI;
  /* readonly */ public string $textContent;

  // methods
  public function __construct();
  public function appendChild<T as DOMNode>(T $newnode): T;
  public function cloneNode(bool $deep = false): this;
  public function getLineNo(): int;
  public function hasAttributes(): bool;
  public function hasChildNodes(): bool;
  public function insertBefore<T as DOMNode>(
    T $newnode,
    ?DOMNode $refnode = null,
  ): T;
  public function isDefaultNamespace(string $namespaceuri): bool;
  public function isSameNode(DOMNode $node): bool;
  public function isSupported(string $feature, string $version): bool;
  public function lookupNamespaceUri(?string $prefix): string;
  public function lookupPrefix(string $namespaceURI): string;
  public function normalize(): void;
  public function removeChild($node): DOMNode;
  public function replaceChild<T as DOMNode>(DOMNode $newchildobj, T $oldchildobj): T;
  public function C14N(bool $exclusive = false,
                       bool $with_comments = false,
                       /* HH_IGNORE_ERROR[2071] */
                       ?darray $xpath = null,
                       /* HH_IGNORE_ERROR[2071] */
                       ?varray $ns_prefixes = null): string;
  public function C14NFile(string $uri,
                           bool $exclusive = false,
                           bool $with_comments = false,
                           /* HH_IGNORE_ERROR[2071] */
                           ?darray $xpath = null,
                           /* HH_IGNORE_ERROR[2071] */
                           ?varray $ns_prefixes = null): int;
  public function getNodePath();
}

class DOMDocument extends DOMNode {

  // properties
  // /* readonly */ public string $actualEncoding; // deprecated
  /* readonly */ public DOMDocumentType $doctype;
  /* readonly */ public DOMElement $documentElement;
  public string $documentURI;
  public string $encoding;
  public bool $formatOutput;
  /* readonly */ public DOMImplementation $implementation;
  public bool $preserveWhiteSpace = true;
  public bool $recover;
  public bool $resolveExternals;
  public bool $standalone;
  public bool $strictErrorChecking = true;
  public bool $substituteEntities;
  public bool $validateOnParse = false;
  public string $version;
  /* readonly */ public string $xmlEncoding;
  public bool $xmlStandalone;
  public string $xmlVersion;

  // methods
  public function __construct($version = null, $encoding = null);
  public function createAttribute(string $name);
  public function createAttributeNS(string $namespaceuri, string $qualifiedname);
  public function createCDATASection(string $data);
  public function createComment(string $data);
  public function createDocumentFragment();
  public function createElement(string $name, $value = null);
  public function createElementNS(string $namespaceuri, string $qualifiedname, $value = null);
  public function createEntityReference(string $name);
  public function createProcessingInstruction(string $target, $data = null);
  public function createTextNode(string $data);
  public function getElementById(string $elementid);
  public function getElementsByTagName(string $name): DOMNodeList<DOMElement>;
  public function getElementsByTagNameNS(
    string $namespaceuri,
    string $localname,
  ): DOMNodeList<DOMElement>;
  public function importNode($importednode, bool $deep = false);
  public function load($filename, $options = 0);
  public function loadHTML($source, $options = 0);
  public function loadHTMLFile($filename, $options = 0);
  public function loadXML($source, $options = 0);
  public function normalizeDocument();
  public function registerNodeClass(string $baseclass, string $extendedclass);
  public function relaxNGValidate(string $filename);
  public function relaxNGValidateSource(string $source);
  public function save(string $file, int $options = 0);
  public function saveHTML($node = null);
  public function saveHTMLFile(string $file);
  public function saveXML($node = null, int $options = 0);
  public function schemaValidate(string $filename);
  public function schemaValidateSource(string $source);
  public function validate();
  public function xinclude(int $options = 0);
}

class DOMElement extends DOMNode {
  // properties
  /* readonly */ public string $tagName;
  /* readonly */ public bool $schemaTypeInfo;

  /* HH_FIXME[4110]: $attributes is nullable in the parent but not here */
  /* HH_FIXME[4341]: $attributes is nullable in the parent but not here */
  /* readonly */ public DOMNamedNodeMap<DOMAttr> $attributes;

  // methods
  public function __construct(string $name, $value = null, $namespaceuri = null);
  public function getAttribute(string $name);
  public function getAttributeNode(string $name);
  public function getAttributeNodeNS(string $namespaceuri, string $localname);
  public function getAttributeNS(string $namespaceuri, string $localname);
  public function getElementsByTagName(string $name);
  public function getElementsByTagNameNS(string $namespaceuri, string $localname);
  public function hasAttribute(string $name);
  public function hasAttributeNS(string $namespaceuri, string $localname);
  public function removeAttribute(string $name);
  public function removeAttributeNode($oldattr);
  public function removeAttributeNS(string $namespaceuri, string $localname);
  public function setAttribute(string $name, string $value);
  public function setAttributeNode($newattr);
  public function setAttributeNodeNS($newattr);
  public function setAttributeNS(string $namespaceuri, string $name, string $value);
  public function setIDAttribute(string $name, bool $isid);
  public function setIDAttributeNode($idattr, bool $isid);
  public function setIDAttributeNS(string $namespaceuri, string $localname, bool $isid);
}

class DOMAttr extends DOMNode {
  // Properties
  /* readonly */ public string $name;
  /* readonly */ public DOMElement $ownerElement;
  /* readonly */ public ?bool $schemaTypeInfo;
  /* readonly */ public ?bool $specified;
  public ?string $value;

  // Methods
  public function __construct(string $name, ?string $value = null);
  public function isId(): bool;
}


abstract class DOMCharacterData extends DOMNode {

  // Properties
  public string $data;
  /* readonly */ public int $length;

  // Methods
  public function appendData(string $data ): void;
  public function deleteData(int $offset, int $count): void;
  public function insertData(int $offset, string $data): void;
  public function replaceData(int $offset, int $count, string $data): void;
  public function substringData(int $offset, int $count): string;
}

class DOMText extends DOMCharacterData {

  // Properties
  /* readonly */ public string $wholeText;

  // Methods
  public function __construct(string $value='');
  public function isWhitespaceInElementContent(): bool;
  public function splitText(int $offset): DOMText;
}

class DOMDocumentType extends DOMNode {

  /* readonly */ public string $publicId;
  /* readonly */ public string $systemId;
  /* readonly */ public string $name;
  /* readonly */ public DOMNamedNodeMap<DOMNode> $entities;
  /* readonly */ public DOMNamedNodeMap<DOMNode> $notations;
  /* readonly */ public string $internalSubset;

  // Methods
  <<__PHPStdLib>> public function __debuginfo(): array;

}

class DOMImplementation {

  // Methods
  public function __construct();
  public function createDocument(
    $namespaceuri = null,
    $qualifiedname = null,
    $doctypeobj = null,
  ): mixed; // DOMDocument or false
  public function createDocumentType(
    $qualifiedname = null,
    $publicid = null,
    $systemid = null,
  ): mixed; // DOMDocumentType or false
  public function hasFeature(string $feature, string $version): bool;

}

class DOMDocumentFragment extends DOMNode {

  // Methods
  public function __construct();
  public function appendXML(string $data): bool;

}

class DOMComment extends DOMCharacterData {
  // Methods
  public function __construct(?string $value = null);
}

<<__PHPStdLib>>
function dom_import_simplexml(SimpleXMLElement $node): ?DOMElement;
<<__PHPStdLib>>
function simplexml_import_dom(
  DOMNode $node,
  string $class_name = 'SimpleXMLElement',
);
