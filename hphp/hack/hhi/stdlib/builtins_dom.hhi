<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int XML_ATTRIBUTE_DECL_NODE;
const int XML_ATTRIBUTE_NODE;
const int XML_CDATA_SECTION_NODE;
const int XML_COMMENT_NODE;
const int XML_DOCUMENT_FRAG_NODE;
const int XML_DOCUMENT_NODE;
const int XML_DOCUMENT_TYPE_NODE;
const int XML_DTD_NODE;
const int XML_ELEMENT_DECL_NODE;
const int XML_ELEMENT_NODE;
const int XML_ENTITY_DECL_NODE;
const int XML_ENTITY_NODE;
const int XML_ENTITY_REF_NODE;
const int XML_HTML_DOCUMENT_NODE;
const int XML_NAMESPACE_DECL_NODE;
const int XML_NOTATION_NODE;
const int XML_PI_NODE;
const int XML_TEXT_NODE;

const int XML_LOCAL_NAMESPACE;

const int XML_ATTRIBUTE_CDATA;
const int XML_ATTRIBUTE_ENTITY;
const int XML_ATTRIBUTE_ENUMERATION;
const int XML_ATTRIBUTE_ID;
const int XML_ATTRIBUTE_IDREF;
const int XML_ATTRIBUTE_IDREFS;
const int XML_ATTRIBUTE_NMTOKEN;
const int XML_ATTRIBUTE_NMTOKENS;
const int XML_ATTRIBUTE_NOTATION;

const int DOM_PHP_ERR;
const int DOM_INDEX_SIZE_ERR;
const int DOMSTRING_SIZE_ERR;
const int DOM_HIERARCHY_REQUEST_ERR;
const int DOM_WRONG_DOCUMENT_ERR;
const int DOM_INVALID_CHARACTER_ERR;
const int DOM_NO_DATA_ALLOWED_ERR;
const int DOM_NO_MODIFICATION_ALLOWED_ERR;
const int DOM_NOT_FOUND_ERR;
const int DOM_NOT_SUPPORTED_ERR;
const int DOM_INUSE_ATTRIBUTE_ERR;
const int DOM_INVALID_STATE_ERR;
const int DOM_SYNTAX_ERR;
const int DOM_INVALID_MODIFICATION_ERR;
const int DOM_NAMESPACE_ERR;
const int DOM_INVALID_ACCESS_ERR;
const int DOM_VALIDATION_ERR;

class DOMXPath {
  public function __construct(HH\FIXME\MISSING_PARAM_TYPE $doc);
  public function evaluate(
    string $expr,
    HH\FIXME\MISSING_PARAM_TYPE $context = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function query(
    string $expr,
    HH\FIXME\MISSING_PARAM_TYPE $context = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function registerNamespace(
    string $prefix,
    string $uri,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function registerPHPFunctions(
    HH\FIXME\MISSING_PARAM_TYPE $funcs = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
}

class DOMNodeList<+Tnode as DOMNode> implements IteratorAggregate<Tnode> {
  /* Properties */
  /* readonly */ public int $length;

  // Methods
  public function item(int $index): Tnode;

  /**
   * Actually returns DOMNodeIterator which is not exposed as an HHI
   */
  public function getIterator(): Iterator<Tnode>;
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
  /* readonly */ public HH\FIXME\MISSING_PROP_TYPE $parentNode;
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
  public function removeChild(HH\FIXME\MISSING_PARAM_TYPE $node): DOMNode;
  public function replaceChild<T as DOMNode>(
    DOMNode $newchildobj,
    T $oldchildobj,
  ): T;
  public function C14N(
    bool $exclusive = false,
    bool $with_comments = false,
    ?darray<arraykey, mixed> $xpath = null,
    ?varray<mixed> $ns_prefixes = null,
  ): string;
  public function C14Nfile(
    string $uri,
    bool $exclusive = false,
    bool $with_comments = false,
    ?darray<arraykey, mixed> $xpath = null,
    ?varray<mixed> $ns_prefixes = null,
  ): int;
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
  public function __construct(
    HH\FIXME\MISSING_PARAM_TYPE $version = null,
    HH\FIXME\MISSING_PARAM_TYPE $encoding = null,
  );
  public function createAttribute(string $name): HH\FIXME\MISSING_RETURN_TYPE;
  public function createAttributeNS(
    string $namespaceuri,
    string $qualifiedname,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function createCDATASection(
    string $data,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function createComment(string $data): HH\FIXME\MISSING_RETURN_TYPE;
  public function createDocumentFragment(): HH\FIXME\MISSING_RETURN_TYPE;
  public function createElement(
    string $name,
    HH\FIXME\MISSING_PARAM_TYPE $value = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function createElementNS(
    string $namespaceuri,
    string $qualifiedname,
    HH\FIXME\MISSING_PARAM_TYPE $value = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function createEntityReference(
    string $name,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function createProcessingInstruction(
    string $target,
    HH\FIXME\MISSING_PARAM_TYPE $data = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function createTextNode(string $data): HH\FIXME\MISSING_RETURN_TYPE;
  public function getElementById(
    string $elementid,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getElementsByTagName(string $name): DOMNodeList<DOMElement>;
  public function getElementsByTagNameNS(
    string $namespaceuri,
    string $localname,
  ): DOMNodeList<DOMElement>;
  public function importNode(
    HH\FIXME\MISSING_PARAM_TYPE $importednode,
    bool $deep = false,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function load(
    HH\FIXME\MISSING_PARAM_TYPE $filename,
    HH\FIXME\MISSING_PARAM_TYPE $options = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function loadHTML(
    HH\FIXME\MISSING_PARAM_TYPE $source,
    HH\FIXME\MISSING_PARAM_TYPE $options = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function loadHTMLFile(
    HH\FIXME\MISSING_PARAM_TYPE $filename,
    HH\FIXME\MISSING_PARAM_TYPE $options = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function loadXML(
    HH\FIXME\MISSING_PARAM_TYPE $source,
    HH\FIXME\MISSING_PARAM_TYPE $options = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function normalizeDocument(): HH\FIXME\MISSING_RETURN_TYPE;
  public function registerNodeClass(
    string $baseclass,
    string $extendedclass,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function relaxNGValidate(
    string $filename,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function relaxNGValidateSource(
    string $source,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function save(
    string $file,
    int $options = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function saveHTML(
    HH\FIXME\MISSING_PARAM_TYPE $node = null,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function saveHTMLFile(string $file): HH\FIXME\MISSING_RETURN_TYPE;
  public function saveXML(
    HH\FIXME\MISSING_PARAM_TYPE $node = null,
    int $options = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function schemaValidate(
    string $filename,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function schemaValidateSource(
    string $source,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function validate(): HH\FIXME\MISSING_RETURN_TYPE;
  public function xinclude(int $options = 0): HH\FIXME\MISSING_RETURN_TYPE;
}

class DOMElement extends DOMNode {
  // properties
  /* readonly */ public string $tagName;
  /* readonly */ public bool $schemaTypeInfo;

  // $attributes is nullable in the parent but not here, however this violates
  // hierarchy rules so users must handle the nullability themselves
  /* public DOMNamedNodeMap<DOMAttr> $attributes; */

  // methods
  public function __construct(
    string $name,
    HH\FIXME\MISSING_PARAM_TYPE $value = null,
    HH\FIXME\MISSING_PARAM_TYPE $namespaceuri = null,
  );
  public function getAttribute(string $name): HH\FIXME\MISSING_RETURN_TYPE;
  public function getAttributeNode(string $name): HH\FIXME\MISSING_RETURN_TYPE;
  public function getAttributeNodeNS(
    string $namespaceuri,
    string $localname,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getAttributeNS(
    string $namespaceuri,
    string $localname,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function getElementsByTagName(string $name): DOMNodeList<DOMElement>;
  public function getElementsByTagNameNS(
    string $namespaceuri,
    string $localname,
  ): DOMNodeList<DOMElement>;
  public function hasAttribute(string $name): HH\FIXME\MISSING_RETURN_TYPE;
  public function hasAttributeNS(
    string $namespaceuri,
    string $localname,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function removeAttribute(string $name): HH\FIXME\MISSING_RETURN_TYPE;
  public function removeAttributeNode(
    HH\FIXME\MISSING_PARAM_TYPE $oldattr,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function removeAttributeNS(
    string $namespaceuri,
    string $localname,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setAttribute(
    string $name,
    string $value,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setAttributeNode(
    HH\FIXME\MISSING_PARAM_TYPE $newattr,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setAttributeNodeNS(
    HH\FIXME\MISSING_PARAM_TYPE $newattr,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setAttributeNS(
    string $namespaceuri,
    string $name,
    string $value,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setIDAttribute(
    string $name,
    bool $isid,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setIDAttributeNode(
    HH\FIXME\MISSING_PARAM_TYPE $idattr,
    bool $isid,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function setIDAttributeNS(
    string $namespaceuri,
    string $localname,
    bool $isid,
  ): HH\FIXME\MISSING_RETURN_TYPE;
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
  public function appendData(string $data): void;
  public function deleteData(int $offset, int $count): void;
  public function insertData(int $offset, string $data): void;
  public function replaceData(int $offset, int $count, string $data): void;
  public function substringData(int $offset, int $count): string;
}

class DOMText extends DOMCharacterData {

  // Properties
  /* readonly */ public string $wholeText;

  // Methods
  public function __construct(string $value = '');
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
  <<__PHPStdLib>>
  public function __debugInfo(): darray<arraykey, mixed>;

}

class DOMImplementation {

  // Methods
  public function __construct();
  public function createDocument(
    HH\FIXME\MISSING_PARAM_TYPE $namespaceuri = null,
    HH\FIXME\MISSING_PARAM_TYPE $qualifiedname = null,
    HH\FIXME\MISSING_PARAM_TYPE $doctypeobj = null,
  ): mixed; // DOMDocument or false
  public function createDocumentType(
    HH\FIXME\MISSING_PARAM_TYPE $qualifiedname = null,
    HH\FIXME\MISSING_PARAM_TYPE $publicid = null,
    HH\FIXME\MISSING_PARAM_TYPE $systemid = null,
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

class DOMCdataSection extends DOMText {
  // Methods
  public function __construct(string $value = '');
}

<<__PHPStdLib>>
function dom_import_simplexml(SimpleXMLElement $node): ?DOMElement;
<<__PHPStdLib>>
function simplexml_import_dom(
  DOMNode $node,
  string $class_name = 'SimpleXMLElement',
): HH\FIXME\MISSING_RETURN_TYPE;
