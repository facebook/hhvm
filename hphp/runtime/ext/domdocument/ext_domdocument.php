<?hh

<<__NativeData>>
class DOMNode {

  public function __construct(): void {
  }

  /**
   * This functions appends a child to an existing list of children or creates
   *   a new list of children. The child can be created with e.g.
   *   DOMDocument::createElement(), DOMDocument::createTextNode() etc. or
   *   simply by using any other node.
   *
   * @param DOMNode $newnode - The appended child.
   *
   * @return mixed - The node added.
   *
   */
  <<__Native>>
  public function appendChild(DOMNode $newnode): mixed;

  /**
   * Creates a copy of the node.
   *
   * @param bool $deep - Indicates whether to copy all descendant nodes. This
   *   parameter is defaulted to FALSE.
   *
   * @return mixed - The cloned node.
   *
   */
  <<__Native>>
  public function cloneNode(bool $deep = false): mixed;

  /**
   * Gets line number for where the node is defined.
   *
   * @return int - Always returns the line number where the node was defined
   *   in.
   *
   */
  <<__Native>>
  public function getLineNo(): int;

  /**
   * This method checks if the node has attributes. The tested node have to be
   *   an XML_ELEMENT_NODE.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function hasAttributes(): bool;

  /**
   * This function checks if the node has children.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function hasChildNodes(): bool;

  /**
   * This function inserts a new node right before the reference node. If you
   *   plan to do further modifications on the appended child you must use the
   *   returned node.
   *
   * @param DOMNode $newnode - The new node.
   * @param DOMNode $refnode - The reference node. If not supplied, newnode is
   *   appended to the children.
   *
   * @return mixed - The inserted node.
   *
   */
  <<__Native>>
  public function insertBefore(DOMNode $newnode, ?DOMNode $refnode = null): mixed;

  /**
   * Tells whether namespaceURI is the default namespace.
   *
   * @param string $namespaceuri - The namespace URI to look for.
   *
   * @return bool - Return TRUE if namespaceURI is the default namespace,
   *   FALSE otherwise.
   *
   */
  <<__Native>>
  public function isDefaultNamespace(string $namespaceuri): bool;

  /**
   * This function indicates if two nodes are the same node. The comparison is
   *   not based on content
   *
   * @param DOMNode $node - The compared node.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function isSameNode(DOMNode $node): bool;

  /**
   * Checks if the asked feature is supported for the specified version.
   *
   * @param string $feature - The feature to test. See the example of
   *   DOMImplementation::hasFeature() for a list of features.
   *
   * @param string $version - The version number of the feature to test.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function isSupported(string $feature, string $version): bool;

  /**
   * Gets the namespace URI of the node based on the prefix.
   *
   * @param mixed $namespaceuri - The prefix of the namespace.
   *
   * @return mixed - The namespace URI of the node.
   *
   */
  <<__Native>>
  public function lookupNamespaceUri(mixed $namespaceuri): mixed;

  /**
   * Gets the namespace prefix of the node based on the namespace URI.
   *
   * @param string $prefix - The namespace URI.
   *
   * @return mixed - The prefix of the namespace.
   *
   */
  <<__Native>>
  public function lookupPrefix(string $prefix): mixed;

  /**
   * Normalizes the node.
   *
   */
  <<__Native>>
  public function normalize(): void;

  /**
   * This functions removes a child from a list of children.
   *
   * @param DOMNode $node - The removed child.
   *
   * @return mixed - If the child could be removed the functions returns the
   *   old child.
   *
   */
  <<__Native>>
  public function removeChild(DOMNode $node): mixed;

  /**
   * This function replaces the child oldnode with the passed new node. If the
   *   new node is already a child it will not be added a second time. If the
   *   replacement succeeds the old node is returned.
   *
   * @param DOMNode $newchildobj - The new node. It must be a member of the
   *   target document, i.e. created by one of the DOMDocument->createXXX()
   *   methods or imported in the document by DOMDocument::importNode.
   * @param DOMNode $oldchildobj - The old node.
   *
   * @return mixed - The old node or FALSE if an error occur.
   *
   */
  <<__Native>>
  public function replaceChild(DOMNode $newchildobj, DOMNode $oldchildobj): mixed;

  <<__Native>>
  public function C14N(bool $exclusive = false,
                bool $with_comments = false,
                mixed $xpath = null,
                mixed $ns_prefixes = null): mixed;

  <<__Native>>
  public function C14Nfile(string $uri,
                    bool $exclusive = false,
                    bool $with_comments = false,
                    mixed $xpath = null,
                    mixed $ns_prefixes = null): mixed;

  <<__Native>>
  public function getNodePath(): mixed;

  /**
   * @return array - var_dump() compat output helper.
   *
   */
  <<__Native>>
  public function __debugInfo(): darray<string, mixed>;
}

/**
 * DOMAttr represents an attribute in the DOMElement object.
 *
 */
class DOMAttr extends DOMNode {

  <<__Native>>
  public function __construct(string $name, ?string $value = null): void;

  /**
   * This function checks if the attribute is a defined ID.  According to the
   *   DOM standard this requires a DTD which defines the attribute ID to be of
   *   type ID. You need to validate your document with DOMDocument::validate or
   *   DOMDocument::validateOnParse before using this function.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function isId(): bool;

  /**
   * @return array - var_dump() compat output helper.
   *
   */
  <<__Native>>
  public function __debugInfo(): darray<string, mixed>;
}

/**
 * Represents nodes with character data. No nodes directly correspond to this
 *   class, but other nodes do inherit from it.
 *
 */
class DOMCharacterData extends DOMNode {

  /**
   * Append the string data to the end of the character data of the node.
   *
   * @param string $arg - The string to append.
   *
   * @return bool - No value is returned.
   *
   */
  <<__Native>>
  public function appendData(string $arg): bool;

  /**
   * Deletes count characters starting from position offset.
   *
   * @param int $offset - The offset from which to start removing.
   * @param int $count - The number of characters to delete. If the sum of
   *   offset and count exceeds the length, then all characters to the end of
   *   the data are deleted.
   *
   * @return bool - No value is returned.
   *
   */
  <<__Native>>
  public function deleteData(int $offset, int $count): bool;

  /**
   * Inserts string data at position offset.
   *
   * @param int $offset - The character offset at which to insert.
   * @param string $data - The string to insert.
   *
   * @return bool - No value is returned.
   *
   */
  <<__Native>>
  public function insertData(int $offset, string $data): bool;

  /**
   * Replace count characters starting from position offset with data.
   *
   * @param int $offset - The offset from which to start replacing.
   * @param int $count - The number of characters to replace. If the sum of
   *   offset and count exceeds the length, then all characters to the end of
   *   the data are replaced.
   * @param string $data - The string with which the range must be replaced.
   *
   * @return bool - No value is returned.
   *
   */
  <<__Native>>
  public function replaceData(int $offset, int $count, string $data): bool;

  /**
   * Returns the specified substring.
   *
   * @param int $offset - Start offset of substring to extract.
   * @param int $count - The number of characters to extract.
   *
   * @return string - The specified substring. If the sum of offset and count
   *   exceeds the length, then all 16-bit units to the end of the data are
   *   returned.
   *
   */
  <<__Native>>
  public function substringData(int $offset, int $count): string;

  /**
   * @return array - var_dump() compat output helper.
   *
   */
  <<__Native>>
  public function __debugInfo(): darray<string, mixed>;
}

/**
 * Represents comment nodes, characters delimited by <!-- and -->.
 *
 */
class DOMComment extends DOMCharacterData {

  <<__Native>>
  public function __construct(?string $value = null): void;
}

/**
 * The DOMText class inherits from DOMCharacterData and represents the textual
 *   content of a DOMElement or DOMAttr.
 *
 */
class DOMText extends DOMCharacterData {

  <<__Native>>
  public function __construct(?string $value = null): void;

  /**
   * Indicates whether this text node contains whitespace. The text node is
   *   determined to contain whitespace in element content during the load of
   *   the document.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function isWhitespaceInElementContent(): bool;

  /**
   * Indicates whether this text node contains whitespace. The text node is
   *   determined to contain whitespace in element content during the load of
   *   the document.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function isElementContentWhitespace(): bool;

  /**
   * Breaks this node into two nodes at the specified offset, keeping both in
   *   the tree as siblings.  After being split, this node will contain all the
   *   content up to the offset. If the original node had a parent node, the new
   *   node is inserted as the next sibling of the original node. When the
   *   offset is equal to the length of this node, the new node has no data.
   *
   * @param int $offset - The offset at which to split, starting from 0.
   *
   * @return mixed - The new node of the same type, which contains all the
   *   content at and after the offset.
   *
   */
  <<__Native>>
  public function splitText(int $offset): mixed;

  /**
   * @return array - var_dump() compat output helper.
   *
   */
  <<__Native>>
  public function __debugInfo(): darray<string, mixed>;
}

class DOMCdataSection extends DOMText {

  <<__Native>>
  public function __construct(string $value): void;
}

/**
 * Represents an entire HTML or XML document; serves as the root of the
 *   document tree.
 *
 */
class DOMDocument extends DOMNode {

  <<__Native("NoRecording")>>
  public function __construct(?string $version = null, ?string $encoding = null): void;

  /**
   * This function creates a new instance of class DOMAttr. This node will not
   *   show up in the document unless it is inserted with (e.g.)
   *   DOMNode->appendChild().
   *
   * @param string $name - The name of the attribute.
   *
   * @return mixed - The new DOMAttr or FALSE if an error occurred.
   *
   */
  <<__Native>>
  public function createAttribute(string $name): mixed;

  /**
   * This function creates a new instance of class DOMAttr. This node will not
   *   show up in the document unless it is inserted with (e.g.)
   *   DOMNode->appendChild().
   *
   * @param string $namespaceuri - The URI of the namespace.
   * @param string $qualifiedname - The tag name and prefix of the attribute,
   *   as prefix:tagname.
   *
   * @return mixed - The new DOMAttr or FALSE if an error occurred.
   *
   */
  <<__Native>>
  public function createAttributeNS(string $namespaceuri,
                             string $qualifiedname): mixed;

  /**
   * This function creates a new instance of class DOMCDATASection. This node
   *   will not show up in the document unless it is inserted with (e.g.)
   *   DOMNode->appendChild().
   *
   * @param string $data - The content of the cdata.
   *
   * @return mixed - The new DOMCDATASection or FALSE if an error occurred.
   *
   */
  <<__Native>>
  public function createCDATASection(string $data): mixed;

  /**
   * This function creates a new instance of class DOMComment. This node will
   *   not show up in the document unless it is inserted with (e.g.)
   *   DOMNode->appendChild().
   *
   * @param string $data - The content of the comment.
   *
   * @return mixed - The new DOMComment or FALSE if an error occurred.
   *
   */
  <<__Native>>
  public function createComment(string $data): mixed;

  /**
   * This function creates a new instance of class DOMDocumentFragment. This
   *   node will not show up in the document unless it is inserted with (e.g.)
   *   DOMNode->appendChild().
   *
   * @return mixed - The new DOMDocumentFragment or FALSE if an error occurred.
   *
   */
  <<__Native>>
  public function createDocumentFragment(): mixed;

  /**
   * This function creates a new instance of class DOMElement. This node will
   *   not show up in the document unless it is inserted with (e.g.)
   *   DOMNode->appendChild().
   *
   * @param string $name - The tag name of the element.
   * @param string $value - The value of the element. By default, an empty
   *   element will be created. The value can also be set later with
   *   DOMElement->nodeValue.
   *
   * @return mixed - Returns a new instance of class DOMElement or FALSE if an
   *   error occurred.
   *
   */
  <<__Native>>
  public function createElement(string $name, ?string $value = null): mixed;

  /**
   * This function creates a new element node with an associated namespace.
   *   This node will not show up in the document unless it is inserted with
   *   (e.g.) DOMNode->appendChild().
   *
   * @param string $namespaceuri - The URI of the namespace.
   * @param string $qualifiedname - The qualified name of the element, as
   *   prefix:tagname.
   * @param string $value - The value of the element. By default, an empty
   *   element will be created. You can also set the value later with
   *   DOMElement->nodeValue.
   *
   * @return mixed - The new DOMElement or FALSE if an error occurred.
   *
   */
  <<__Native>>
  public function createElementNS(string $namespaceuri,
                           string $qualifiedname,
                           ?string $value = null): mixed;

  /**
   * This function creates a new instance of class DOMEntityReference. This
   *   node will not show up in the document unless it is inserted with (e.g.)
   *   DOMNode->appendChild().
   *
   * @param string $name - The content of the entity reference, e.g. the
   *   entity reference minus the leading & and the trailing ; characters.
   *
   * @return mixed - The new DOMEntityReference or FALSE if an error occurred.
   *
   */
  <<__Native>>
  public function createEntityReference(string $name): mixed;

  /**
   * This function creates a new instance of class DOMProcessingInstruction.
   *   This node will not show up in the document unless it is inserted with
   *   (e.g.) DOMNode->appendChild().
   *
   * @param string $target - The target of the processing instruction.
   * @param string $data - The content of the processing instruction.
   *
   * @return mixed - The new DOMProcessingInstruction or FALSE if an error
   *   occurred.
   *
   */
  <<__Native>>
  public function createProcessingInstruction(string $target,
                                       ?string $data = null): mixed;

  /**
   * This function creates a new instance of class DOMText. This node will not
   *   show up in the document unless it is inserted with (e.g.)
   *   DOMNode->appendChild().
   *
   * @param string $data - The content of the text.
   *
   * @return mixed - The new DOMText or FALSE if an error occurred.
   *
   */
  <<__Native>>
  public function createTextNode(string $data): mixed;

  /**
   * This function is similar to DOMDocument::getElementsByTagName but
   *   searches for an element with a given id.  For this function to work, you
   *   will need either to set some ID attributes with
   *   DOMElement::setIdAttribute or a DTD which defines an attribute to be of
   *   type ID. In the later case, you will need to validate your document with
   *   DOMDocument::validate or DOMDocument->validateOnParse before using this
   *   function.
   *
   * @param string $elementid - The unique id value for an element.
   *
   * @return mixed - Returns the DOMElement or NULL if the element is not
   *   found.
   *
   */
  <<__Native>>
  public function getElementById(string $elementid): mixed;

  /**
   * This function returns a new instance of class DOMNodeList containing the
   *   elements with a given tag name.
   *
   * @param string $name - The name of the tag to match on. The special value
   *   * matches all tags.
   *
   * @return DOMNodeList - A new DOMNodeList object containing all the matched
   *   elements.
   *
   */
  <<__Native>>
  public function getElementsByTagName(string $name): DOMNodeList;

  /**
   * Returns a DOMNodeList of all elements with a given local name and a
   *   namespace URI.
   *
   * @param string $namespaceuri - The namespace URI of the elements to match
   *   on. The special value * matches all namespaces.
   * @param string $localname - The local name of the elements to match on.
   *   The special value * matches all local names.
   *
   * @return DOMNodeList - A new DOMNodeList object containing all the matched
   *   elements.
   *
   */
  <<__Native>>
  public function getElementsByTagNameNS(
    string $namespaceuri,
    string $localname,
  ): DOMNodeList;

  /**
   * This function returns a copy of the node to import and associates it with
   *   the current document.
   *
   * @param DOMNode $importednode - The node to import.
   * @param bool $deep - If set to TRUE, this method will recursively import
   *   the subtree under the importedNode.  To copy the nodes attributes deep
   *   needs to be set to TRUE
   *
   * @return mixed - The copied node or FALSE, if it cannot be copied.
   *
   */
  <<__Native>>
  public function importNode(DOMNode $importednode, bool $deep = false): mixed;

  /**
   * Internal helper function for load()/loadXML()
   */
  <<__Native("NoRecording")>>
  private function _load(string $arg, int $options,
                         bool $isFile): bool;

  /**
   * Internal helper function for loadHTMLFile()/loadHTML()
   */
  <<__Native>>
  private function _loadHTML(string $arg, int $options,
                             bool $isFile): bool;

  /**
   * Loads an XML document from a file. Warning Unix style paths with forward
   *   slashes can cause significant performance degradation on Windows systems;
   *   be sure to call realpath() in such a case.
   *
   * @param string $filename - The path to the XML document.
   * @param int $options - Bitwise OR of the libxml option constants.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   */
  public function load(string $filename, int $options = 0): bool {
    return $this->_load($filename, $options, true);
  }

  /**
   * The function parses the HTML contained in the string source. Unlike
   *   loading XML, HTML does not have to be well-formed to load.
   *
   * @param string $source - The HTML string.
   * @param int $options - Since PHP 5.4.0 and libxml 2.6.0, you may also use
   *   the options parameter to specify additional Libxml parameters
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   */
  public function loadHTML(string $source, int $options = 0): bool {
    return $this->_loadHTML($source, $options, false);
  }


  /**
   * The function parses the HTML document in the file named filename. Unlike
   *   loading XML, HTML does not have to be well-formed to load.
   *
   * @param string $filename - The path to the HTML file.
   * @param int $options - Since PHP 5.4.0 and libxml 2.6.0, you may also use
   *   the options parameter to specify additional Libxml parameters
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   */
  public function loadHTMLFile(string $filename, int $options = 0): bool {
    return $this->_loadHTML($filename, $options, true);
  }

  /**
   * Loads an XML document from a string.
   *
   * @param string $source - The string containing the XML.
   * @param int $options - Bitwise OR of the libxml option constants.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   */
  public function loadXML(string $source, int $options = 0): bool {
    return $this->_load($source, $options, false);
  }


  /**
   * This method acts as if you saved and then loaded the document, putting
   *   the document in a "normal" form.
   *
   */
  <<__Native>>
  public function normalizeDocument(): void;

  /**
   * This method allows you to register your own extended DOM class to be used
   *   afterward by the PHP DOM extension.  This method is not part of the DOM
   *   standard.
   *
   * @param string $baseclass - The DOM class that you want to extend. You can
   *   find a list of these classes in the chapter introduction.
   * @param string $extendedclass - Your extended class name. If NULL is
   *   provided, any previously registered class extending baseclass will be
   *   removed.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function registerNodeClass(string $baseclass, string $extendedclass): bool;

  /**
   * @param string $filename - The RNG file.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function relaxNGValidate(string $filename): bool;

  /**
   * @param string $source - A string containing the RNG schema.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function relaxNGValidateSource(string $source): bool;

  /**
   * Creates an XML document from the DOM representation. This function is
   *   usually called after building a new dom document from scratch as in the
   *   example below.
   *
   * @param string $file - The path to the saved XML document.
   * @param int $options - Additional Options. Currently only
   *   LIBXML_NOEMPTYTAG is supported.
   *
   * @return mixed - Returns the number of bytes written or FALSE if an error
   *   occurred.
   *
   */
  <<__Native>>
  public function save(string $file, int $options = 0): mixed;

  /**
   * Creates an HTML document from the DOM representation. This function is
   *   usually called after building a new dom document from scratch as in the
   *   example below.
   *
   * @param object $node - Optional parameter to output a subset of the
   *   document.
   *
   * @return mixed - Returns the HTML, or FALSE if an error occurred.
   *
   */
  <<__Native>>
  public function saveHTML(?DOMNode $node = null): mixed;

  /**
   * Creates an HTML document from the DOM representation. This function is
   *   usually called after building a new dom document from scratch as in the
   *   example below.
   *
   * @param string $file - The path to the saved HTML document.
   *
   * @return mixed - Returns the number of bytes written or FALSE if an error
   *   occurred.
   *
   */
  <<__Native>>
  public function saveHTMLFile(string $file): mixed;

  /**
   * Creates an XML document from the DOM representation. This function is
   *   usually called after building a new dom document from scratch as in the
   *   example below.
   *
   * @param DOMNode $node - Use this parameter to output only a specific node
   *   without XML declaration rather than the entire document.
   * @param int $options - Additional Options. Currently only
   *   LIBXML_NOEMPTYTAG is supported.
   *
   * @return mixed - Returns the XML, or FALSE if an error occurred.
   *
   */
  <<__Native>>
  public function saveXML(?DOMNode $node = null, int $options = 0): mixed;

  /**
   * Validates a document based on the given schema file.
   *
   * @param string $filename - The path to the schema.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function schemaValidate(string $filename): bool;

  /**
   * Validates a document based on a schema defined in the given string.
   *
   * @param string $source - A string containing the schema.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function schemaValidateSource(string $source): bool;

  /**
   * Validates the document based on its DTD.  You can also use the
   *   validateOnParse property of DOMDocument to make a DTD validation.
   *
   * @return bool - Returns TRUE on success or FALSE on failure. If the
   *   document have no DTD attached, this method will return FALSE.
   *
   */
  <<__Native>>
  public function validate(): bool;

  /**
   * @param int $options - libxml parameters. Available since PHP 5.1.0 and
   *   Libxml 2.6.7.
   *
   * @return mixed - Returns the number of XIncludes in the document, -1 if
   *   some processing failed, or FALSE if there were no substitutions.
   *
   */
  <<__Native>>
  public function xinclude(int $options = 0): mixed;

  /**
   * @return array - var_dump() compat output helper.
   *
   */
  <<__Native>>
  public function __debugInfo(): darray<string, mixed>;
}

class DOMDocumentFragment extends DOMNode {

  <<__Native>>
  public function __construct(): void;

  /**
   * Appends raw XML data to a DOMDocumentFragment.  This method is not part
   *   of the DOM standard. It was created as a simpler approach for appending
   *   an XML DocumentFragment in a DOMDocument.  If you want to stick to the
   *   standards, you will have to create a temporary DOMDocument with a dummy
   *   root and then loop through the child nodes of the root of your XML data
   *   to append them.
   *
   * @param string $data - XML to append.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function appendXML(string $data): bool;
}

/**
 * Each DOMDocument has a doctype attribute whose value is either NULL or a
 *   DOMDocumentType object.
 *
 */
class DOMDocumentType extends DOMNode {

  /**
   * @return array - var_dump() compat output helper.
   *
   */
  <<__Native>>
  public function __debugInfo(): darray<string, mixed>;
}

<<__NativeData>>
class DOMElement extends DOMNode {

  <<__Native>>
  public function __construct(string $name,
                       ?string $value = null,
                       ?string $namespaceuri = null): void;

  /**
   * Gets the value of the attribute with name name for the current node.
   *
   * @param string $name - The name of the attribute.
   *
   * @return string - The value of the attribute, or an empty string if no
   *   attribute with the given name is found.
   *
   */
  <<__Native>>
  public function getAttribute(string $name): string;

  /**
   * Returns the attribute node with name name for the current element.
   *
   * @param string $name - The name of the attribute.
   *
   * @return mixed - The attribute node.
   *
   */
  <<__Native>>
  public function getAttributeNode(string $name): mixed;

  /**
   * Returns the attribute node in namespace namespaceURI with local name
   *   localName for the current node.
   *
   * @param string $namespaceuri - The namespace URI.
   * @param string $localname - The local name.
   *
   * @return ?DOMNode - The attribute node.
   *
   */
  <<__Native>>
  public function getAttributeNodeNS(
    string $namespaceuri,
    string $localname,
  ): ?DOMNode;

  /**
   * Gets the value of the attribute in namespace namespaceURI with local name
   *   localName for the current node.
   *
   * @param string $namespaceuri - The namespace URI.
   * @param string $localname - The local name.
   *
   * @return string - The value of the attribute, or an empty string if no
   *   attribute with the given localName and namespaceURI is found.
   *
   */
  <<__Native>>
  public function getAttributeNS(string $namespaceuri, string $localname): string;

  /**
   * This function returns a new instance of the class DOMNodeList of all
   *   descendant elements with a given tag name, in the order in which they are
   *   encountered in a preorder traversal of this element tree.
   *
   * @param string $name - The tag name. Use * to return all elements within
   *   the element tree.
   *
   * @return DOMNodeList - This function returns a new instance of the class
   *   DOMNodeList of all matched elements.
   *
   */
  <<__Native>>
  public function getElementsByTagName(string $name): DOMNodeList;

  /**
   * This function fetch all the descendant elements with a given localName
   *   and namespaceURI.
   *
   * @param string $namespaceuri - The namespace URI.
   * @param string $localname - The local name. Use * to return all elements
   *   within the element tree.
   *
   * @return DOMNodeList - This function returns a new instance of the class
   *   DOMNodeList of all matched elements in the order in which they are
   *   encountered in a preorder traversal of this element tree.
   *
   */
  <<__Native>>
  public function getElementsByTagNameNS(
    string $namespaceuri,
    string $localname,
  ): DOMNodeList;

  /**
   * Indicates whether attribute named name exists as a member of the element.
   *
   * @param string $name - The attribute name.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function hasAttribute(string $name): bool;

  /**
   * Indicates whether attribute in namespace namespaceURI named localName
   *   exists as a member of the element.
   *
   * @param string $namespaceuri - The namespace URI.
   * @param string $localname - The local name.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function hasAttributeNS(string $namespaceuri, string $localname): bool;

  /**
   * Removes attribute named name from the element.
   *
   * @param string $name - The name of the attribute.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function removeAttribute(string $name): bool;

  /**
   * Removes attribute oldnode from the element.
   *
   * @param DOMAttr $oldattr - The attribute node.
   *
   * @return mixed - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function removeAttributeNode(DOMAttr $oldattr): mixed;

  /**
   * Removes attribute is namespace namespaceURI named localName from the
   *   element.
   *
   * @param string $namespaceuri - The namespace URI.
   * @param string $localname - The local name.
   *
   * @return mixed - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function removeAttributeNS(string $namespaceuri, string $localname): mixed;

  /**
   * Sets an attribute with name name to the given value. If the attribute
   *   does not exist, it will be created.
   *
   * @param string $name - The name of the attribute.
   * @param string $value - The value of the attribute.
   *
   * @return mixed - The new DOMAttr or FALSE if an error occurred.
   *
   */
  <<__Native>>
  public function setAttribute(string $name, string $value): mixed;

  /**
   * Adds new attribute node attr to element.
   *
   * @param DOMAttr $newattr - The attribute node.
   *
   * @return mixed - Returns old node if the attribute has been replaced or
   *   NULL.
   *
   */
  <<__Native>>
  public function setAttributeNode(DOMAttr $newattr): mixed;

  /**
   * Adds new attribute node attr to element.
   *
   * @param DOMAttr $newattr - The attribute node.
   *
   * @return mixed - Returns the old node if the attribute has been replaced.
   *
   */
  <<__Native>>
  public function setAttributeNodeNS(DOMAttr $newattr): mixed;

  /**
   * Sets an attribute with namespace namespaceURI and name name to the given
   *   value. If the attribute does not exist, it will be created.
   *
   * @param string $namespaceuri - The namespace URI.
   * @param string $name - The qualified name of the attribute, as
   *   prefix:tagname.
   * @param string $value - The value of the attribute.
   *
   * @return mixed - No value is returned.
   *
   */
  <<__Native>>
  public function setAttributeNS(string $namespaceuri,
                          string $name,
                          string $value): mixed;

  /**
   * Declares the attribute name to be of type ID.
   *
   * @param string $name - The name of the attribute.
   * @param bool $isid - Set it to TRUE if you want name to be of type ID,
   *   FALSE otherwise.
   *
   * @return mixed - No value is returned.
   *
   */
  <<__Native>>
  public function setIDAttribute(string $name, bool $isid): mixed;

  /**
   * Declares the attribute specified by attr to be of type ID.
   *
   * @param DOMAttr $idattr - The attribute node.
   * @param bool $isid - Set it to TRUE if you want name to be of type ID,
   *   FALSE otherwise.
   *
   * @return mixed - No value is returned.
   *
   */
  <<__Native>>
  public function setIDAttributeNode(DOMAttr $idattr, bool $isid): mixed;

  /**
   * Declares the attribute specified by localName and namespaceURI to be of
   *   type ID.
   *
   * @param string $namespaceuri - The namespace URI of the attribute.
   * @param string $localname - The local name of the attribute, as
   *   prefix:tagname.
   * @param bool $isid - Set it to TRUE if you want name to be of type ID,
   *   FALSE otherwise.
   *
   * @return mixed - No value is returned.
   *
   */
  <<__Native>>
  public function setIDAttributeNS(string $namespaceuri,
                            string $localname,
                            bool $isid): mixed;

  /**
   * @return array - var_dump() compat output helper.
   *
   */
  <<__Native>>
  public function __debugInfo(): darray<string, mixed>;

}

/**
 * This interface represents a known entity, either parsed or unparsed, in an
 *   XML document.
 *
 */
class DOMEntity extends DOMNode {

  /**
   * @return array - var_dump() compat output helper.
   *
   */
  <<__Native>>
  public function __debugInfo(): darray<string, mixed>;
}

class DOMEntityReference extends DOMNode {

  <<__Native>>
  public function __construct(string $name): void;
}

class DOMNotation extends DOMNode {

  /**
   * @return array - var_dump() compat output helper.
   *
   */
  <<__Native>>
  public function __debugInfo(): darray<string, mixed>;
}

class DOMProcessingInstruction extends DOMNode {

  <<__Native>>
  public function __construct(string $name, ?string $value = null): void;

  /**
   * @return array - var_dump() compat output helper.
   *
   */
  <<__Native>>
  public function __debugInfo(): darray<string, mixed>;
}

class DOMNameSpaceNode extends DOMNode {
}

<<__NativeData>>
class DOMNodeIterator implements Iterator<?DOMNode> {

  public function __construct(): void {
  }

  <<__Native>>
  public function current(): ?DOMNode;

  <<__Native>>
  public function key(): mixed;

  <<__Native>>
  public function next(): void;

  <<__Native>>
  public function rewind(): void;

  <<__Native>>
  public function valid(): bool;
}

<<__NativeData>>
class DOMNamedNodeMap implements IteratorAggregate<?DOMNode> {

  public function __construct()[]: void {
  }

  /**
   * Retrieves a node specified by its nodeName.
   *
   * @param string $name - The nodeName of the node to retrieve.
   *
   * @return mixed - A node (of any type) with the specified nodeName, or NULL
   *   if no node is found.
   *
   */
  <<__Native>>
  public function getNamedItem(string $name): mixed;

  /**
   * Retrieves a node specified by localName and namespaceURI.
   *
   * @param string $namespaceuri - The namespace URI of the node to retrieve.
   * @param string $localname - The local name of the node to retrieve.
   *
   * @return mixed - A node (of any type) with the specified local name and
   *   namespace URI, or NULL if no node is found.
   *
   */
  <<__Native>>
  public function getNamedItemNS(string $namespaceuri, string $localname): mixed;

  /**
   * Retrieves a node specified by index within the DOMNamedNodeMap object.
   *
   * @param int $index - Index into this map.
   *
   * @return mixed - The node at the indexth position in the map, or NULL if
   *   that is not a valid index (greater than or equal to the number of nodes
   *   in this map).
   *
   */
  <<__Native>>
  public function item(int $index): mixed;

  <<__Native>>
  public function getIterator()[]: DOMNodeIterator;
}

<<__NativeData>>
class DOMNodeList implements IteratorAggregate<?DOMNode> {

  public function __construct()[]: void {
  }

  /**
   * Retrieves a node specified by index within the DOMNodeList object. Tip
   *   If you need to know the number of nodes in the collection, use the length
   *   property of the DOMNodeList object.
   *
   * @param int $index - Index of the node into the collection.
   *
   * @return mixed - The node at the indexth position in the DOMNodeList, or
   *   NULL if that is not a valid index.
   *
   */
  <<__Native>>
  public function item(int $index): mixed;

  <<__Native>>
  public function getIterator()[]: DOMNodeIterator;

  /**
   * @return array - var_dump() compat output helper.
   *
   */
  <<__Native>>
  public function __debugInfo(): darray<string, mixed>;
}

/**
 * The DOMImplementation interface provides a number of methods for performing
 *   operations that are independent of any particular instance of the document
 *   object model.
 *
 */
class DOMImplementation {

  public function __construct(): void {
  }

  /**
   * Creates a DOMDocument object of the specified type with its document
   *   element.
   *
   * @param string $namespaceuri - The namespace URI of the document element
   *   to create.
   * @param string $qualifiedname - The qualified name of the document element
   *   to create.
   * @param DOMDocumentType $doctypeobj - The type of document to create or
   *   null.
   *
   * @return mixed - A new DOMDocument object. If namespaceURI, qualifiedName,
   *   and doctype are null, the returned DOMDocument is empty with no document
   *   element
   *
   */
  <<__Native>>
  public function createDocument(?string $namespaceuri = null,
                          ?string $qualifiedname = null,
                          ?DOMDocumentType $doctypeobj = null): mixed;

  /**
   * Creates an empty DOMDocumentType object. Entity declarations and
   *   notations are not made available. Entity reference expansions and default
   *   attribute additions do not occur.
   *
   * @param string $qualifiedname - The qualified name of the document type to
   *   create.
   * @param string $publicid - The external subset public identifier.
   * @param string $systemid - The external subset system identifier.
   *
   * @return mixed - A new DOMDocumentType node with its ownerDocument set to
   *   NULL.
   *
   */
  <<__Native>>
  public function createDocumentType(?string $qualifiedname = null,
                              ?string $publicid = null,
                              ?string $systemid = null): mixed;

  /**
   * @param string $feature - The feature to test.
   * @param string $version - The version number of the feature to test. In
   *   level 2, this can be either 2.0 or 1.0.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function hasFeature(string $feature, string $version): bool;
}

/**
 * Supports XPath 1.0
 *
 */
<<__NativeData>>
class DOMXPath {

  <<__Native>>
  public function __construct(mixed $doc): void;

  /**
   * Executes the given XPath expression and returns a typed result if
   *   possible.
   *
   * @param string $expr - The XPath expression to execute.
   * @param DOMNode $context - The optional contextnode can be specified for
   *   doing relative XPath queries. By default, the queries are relative to the
   *   root element.
   * @param bool $registgerNodeNS - disable automatic registration of the
   *   context node.
   *
   * @return mixed - Returns a typed result if possible or a DOMNodeList
   *   containing all nodes matching the given XPath expression.
   *
   */
  <<__Native>>
  public function evaluate(string $expr,
                    ?DOMNode $context = null,
                    bool $registerNodeNS = true): mixed;

  /**
   * Executes the given XPath expression.
   *
   * @param string $expr - The XPath expression to execute.
   * @param DOMNode $context - The optional contextnode can be specified for
   *   doing relative XPath queries. By default, the queries are relative to the
   *   root element.
   * @param bool $registgerNodeNS - disable automatic registration of the
   *   context node.
   *
   * @return mixed - Returns a DOMNodeList containing all nodes matching the
   *   given XPath expression. Any expression which do not return nodes will
   *   return an empty DOMNodeList.
   *
   */
  <<__Native>>
  public function query(string $expr,
                 ?DOMNode $context = null,
                 bool $registerNodeNS = true): mixed;

  /**
   * Registers the namespaceURI and prefix with the DOMXPath object.
   *
   * @param string $prefix - The prefix.
   * @param string $uri - The URI of the namespace.
   *
   * @return bool - Returns TRUE on success or FALSE on failure.
   *
   */
  <<__Native>>
  public function registerNamespace(string $prefix, string $uri): bool;

  /**
   * This method enables the ability to use PHP functions within XPath
   *   expressions.
   *
   * @param mixed $funcs - Use this parameter to only allow certain functions
   *   to be called from XPath.  This parameter can be either a string (a
   *   function name) or an array of function names.
   *
   * @return mixed - No value is returned.
   *
   */
  <<__Native>>
  public function registerPHPFunctions(mixed $funcs = null): mixed;

  <<__Native>>
  public function __debugInfo(): darray<string, mixed>;
}

function dom_document_create_element(DOMDocument $obj,
                                     string $name,
                                     ?string $value = null): mixed {
  return $obj->createElement($name, $value);
}

function dom_document_create_document_fragment(DOMDocument $obj): mixed {
  return $obj->createDocumentFragment();
}

function dom_document_create_text_node(DOMDocument $obj, string $data): mixed {
  return $obj->createTextNode($data);
}

function dom_document_create_comment(DOMDocument $obj, string $data): mixed {
  return $obj->createComment($data);
}

function dom_document_create_cdatasection(DOMDocument $obj,
                                          string $data): mixed {
  return $obj->createCDATASection($data);
}

function dom_document_create_processing_instruction(DOMDocument $obj,
    string $target, ?string $data = null): mixed {
  return $obj->createProcessingInstruction($target, $data);
}

function dom_document_create_attribute(DOMDocument $obj, string $name): mixed {
  return $obj->createAttribute($name);
}

function dom_document_create_entity_reference(DOMDocument $obj,
                                              string $name): mixed {
  return $obj->createEntityReference($name);
}

function dom_document_get_elements_by_tag_name(
  DOMDocument $obj,
  string $name,
): DOMNodeList {
  return $obj->getElementsByTagName($name);
}

function dom_document_import_node(DOMDocument $obj,
                                  DOMNode $importednode,
                                  bool $deep = false): mixed {
  return $obj->importNode($importednode, $deep);
}

function dom_document_create_element_ns(DOMDocument $obj,
                                        string $namespaceuri,
                                        string $qualifiedname,
                                        ?string $value = null): mixed {
  return $obj->createElementNS($namespaceuri, $qualifiedname, $value);
}

function dom_document_create_attribute_ns(DOMDocument $obj,
                                          string $namespaceuri,
                                          string $qualifiedname): mixed {
  return $obj->createAttributeNS($namespaceuri, $qualifiedname);
}

function dom_document_get_elements_by_tag_name_ns(
  DOMDocument $obj,
  string $namespaceuri,
  string $localname,
): DOMNodeList {
  return $obj->getElementsByTagNameNS($namespaceuri, $localname);
}

function dom_document_get_element_by_id(DOMDocument $obj,
                                        string $elementid): mixed {
  return $obj->getElementById($elementid);
}

function dom_document_normalize_document(DOMDocument $obj): void {
  $obj->normalizeDocument();
}

/**
 * Creates an XML document from the DOM representation. This function is
 *   usually called after building a new dom document from scratch as in the
 *   example below.
 *
 * @param DOMDocument $obj
 * @param string $file - The path to the saved XML document.
 * @param int $options - Additional Options. Currently only
 *   LIBXML_NOEMPTYTAG is supported.
 *
 * @return mixed - Returns the number of bytes written or FALSE if an error
 *   occurred.
 *
 */
function dom_document_save(DOMDocument $obj,
                           string $file,
                           int $options = 0): mixed {
  return $obj->save($file, $options);
}

/**
 * Creates an XML document from the DOM representation. This function is
 *   usually called after building a new dom document from scratch as in the
 *   example below.
 *
 * @param DOMDocument $obj
 * @param DOMNode $node - Use this parameter to output only a specific node
 *   without XML declaration rather than the entire document.
 * @param int $options - Additional Options. Currently only
 *   LIBXML_NOEMPTYTAG is supported.
 *
 * @return mixed - Returns the XML, or FALSE if an error occurred.
 *
 */
function dom_document_savexml(DOMDocument $obj,
                              ?DOMNode $node = null,
                              int $options = 0): mixed {
  return $obj->saveXML($node, $options);
}

/**
 * Validates the document based on its DTD.  You can also use the
 *   validateOnParse property of DOMDocument to make a DTD validation.
 *
 * @return bool - Returns TRUE on success or FALSE on failure. If the
 *   document have no DTD attached, this method will return FALSE.
 *
 */
function dom_document_validate(DOMDocument $obj): bool {
  return $obj->validate();
}

/**
 * @param DOMDocument $obj - libxml parameters. Available since PHP 5.1.0 and
 *   Libxml 2.6.7.
 *
 * @return mixed - Returns the number of XIncludes in the document, -1 if some
 *   processing failed, or FALSE if there were no substitutions.
 *
 */
function dom_document_xinclude(DOMDocument $obj,
                               int $options = 0): mixed {
  return $obj->xinclude($options);
}

/**
 * Creates an HTML document from the DOM representation. This function is
 *   usually called after building a new dom document from scratch as in the
 *   example below.
 *
 * @param DOMDocument $obj
 * @param DOMNode $node - Optional parameter to output a subset of the
 *   document.
 *
 * @return mixed - Returns the HTML, or FALSE if an error occurred.
 *
 */
function dom_document_save_html(DOMDocument $obj,
                                ?DOMNode $node = null): mixed {
  return $obj->saveHTML($node);
}

function dom_document_save_html_file(DOMDocument $obj,
                                     string $file): mixed {
  return $obj->saveHTMLFile($file);
}

function dom_document_schema_validate_file(DOMDocument $obj,
                                           string $filename): bool {
  return $obj->schemaValidate($filename);
}

function dom_document_schema_validate_xml(DOMDocument $obj,
                                          string $source): bool {
  return $obj->schemaValidate($source);
}

function dom_document_relaxng_validate_file(DOMDocument $obj,
                                            string $filename): bool {
  return $obj->relaxNGValidate($filename);
}

function dom_document_relaxng_validate_xml(DOMDocument $obj,
                                           string $source): bool {
  return $obj->relaxNGValidateSource($source);
}

function dom_node_insert_before(DOMNode $obj,
                                DOMNode $newnode,
                                ?DOMNode $refnode = null): mixed {
  return $obj->insertBefore($newnode, $refnode);
}

function dom_node_replace_child(DOMNode $obj,
                                DOMNode $newchildobj,
                                DOMNode $oldchildobj): mixed {
  return $obj->replaceChild($newchildobj, $oldchildobj);
}

function dom_node_remove_child(DOMNode $obj,
                               DOMNode $node): mixed {
  return $obj->removeChild($node);
}

function dom_node_append_child(DOMNode $obj,
                               DOMNode $newnode): mixed {
  return $obj->appendChild($newnode);
}

function dom_node_has_child_nodes(DOMNode $obj): bool {
  return $obj->hasChildNodes();
}

function dom_node_clone_node(DOMNode $obj,
                             bool $deep = false): mixed {
  return $obj->cloneNode($deep);
}

function dom_node_normalize(DOMNode $obj): void {
  $obj->normalize();
}

function dom_node_is_supported(DOMNode $obj,
                               string $feature,
                               string $version): bool {
  return $obj->isSupported($feature, $version);
}

function dom_node_has_attributes(DOMNode $obj): bool {
  return $obj->hasAttributes();
}

function dom_node_is_same_node(DOMNode $obj,
                               DOMNode $node): bool {
  return $obj->isSameNode($node);
}

function dom_node_lookup_prefix(DOMNode $obj,
                                string $prefix): mixed {
  return $obj->lookupPrefix($prefix);
}

function dom_node_is_default_namespace(DOMNode $obj,
                                       string $namespaceuri): bool {
  return $obj->isDefaultNamespace($namespaceuri);
}

function dom_node_lookup_namespace_uri(DOMNode $obj,
                                       string $namespaceuri): mixed {
  return $obj->lookupNamespaceUri($namespaceuri);
}

/**
 * Retrieves a node specified by index within the DOMNodeList object. Tip
 *   If you need to know the number of nodes in the collection, use the length
 *   property of the DOMNodeList object.
 *
 * @param DOMNodeList $obj
 * @param int $index - Index of the node into the collection.
 *
 * @return mixed - The node at the indexth position in the DOMNodeList, or
 *   NULL if that is not a valid index.
 *
 */
function dom_nodelist_item(DOMNodeList $obj, int $index): mixed {
  return $obj->item($index);
}

function dom_namednodemap_get_named_item(DOMNamedNodeMap $obj,
                                         string $name): mixed {
  return $obj->getNamedItem($name);
}

/**
 * Retrieves a node specified by index within the DOMNamedNodeMap object.
 *
 * @param DOMNamedNodeMap $obj
 * @param int $index - Index into this map.
 *
 * @return mixed - The node at the indexth position in the map, or NULL if
 *   that is not a valid index (greater than or equal to the number of nodes
 *   in this map).
 *
 */
function dom_namednodemap_item(DOMNamedNodeMap $obj, int $index): mixed {
  return $obj->item($index);
}

function dom_namednodemap_get_named_item_ns(DOMNamedNodeMap $obj,
                                            string $namespaceuri,
                                            string $localname): mixed {
  return $obj->getNamedItemNS($namespaceuri, $localname);
}

function dom_characterdata_substring_data(DOMCharacterData $obj,
                                          int $offset,
                                          int $count): string {
  return $obj->substringData($offset, $count);
}

function dom_characterdata_append_data(DOMCharacterData $obj,
                                       string $arg): bool {
  return $obj->appendData($arg);
}

function dom_characterdata_insert_data(DOMCharacterData $obj,
                                       int $offset,
                                       string $data): bool {
  return $obj->insertData($offset, $data);
}

function dom_characterdata_delete_data(DOMCharacterData $obj,
                                       int $offset,
                                       int $count): bool {
  return $obj->deleteData($offset, $count);
}

function dom_characterdata_replace_data(DOMCharacterData $obj,
                                        int $offset,
                                        int $count,
                                        string $data): bool {
  return $obj->replaceData($offset, $count, $data);
}

function dom_attr_is_id(DOMAttr $obj): bool {
  return $obj->isId();
}

function dom_element_get_attribute(DOMElement $obj, string $name): string {
  return $obj->getAttribute($name);
}

function dom_element_set_attribute(DOMElement $obj,
                                   string $name,
                                   string $value): mixed {
  return $obj->setAttribute($name, $value);
}

function dom_element_remove_attribute(DOMElement $obj, string $name): bool {
  return $obj->removeAttribute($name);
}

function dom_element_get_attribute_node(DOMElement $obj, string $name): mixed {
  return $obj->getAttributeNode($name);
}

function dom_element_set_attribute_node(DOMElement $obj,
                                        DOMAttr $newattr): mixed {
  return $obj->setAttributeNode($newattr);
}

function dom_element_remove_attribute_node(DOMElement $obj,
                                           DOMAttr $oldattr): mixed {
  return $obj->removeAttributeNode($oldattr);
}

function dom_element_get_elements_by_tag_name(
  DOMElement $obj,
  string $name,
): DOMNodeList {
  return $obj->getElementsByTagName($name);
}

function dom_element_get_attribute_ns(DOMElement $obj,
                                      string $namespaceuri,
                                      string $localname): string {
  return $obj->getAttributeNS($namespaceuri, $localname);
}

function dom_element_set_attribute_ns(DOMElement $obj,
                                      string $namespaceuri,
                                      string $name,
                                      string $value): mixed {
  return $obj->setAttributeNS($namespaceuri, $name, $value);
}

function dom_element_remove_attribute_ns(DOMElement $obj,
                                         string $namespaceuri,
                                         string $localname): mixed {
  return $obj->removeAttributeNS($namespaceuri, $localname);
}

function dom_element_get_attribute_node_ns(DOMElement $obj,
                                           string $namespaceuri,
                                           string $localname): ?DOMNode {
  return $obj->getAttributeNodeNS($namespaceuri, $localname);
}

function dom_element_set_attribute_node_ns(DOMElement $obj,
                                           DOMAttr $newattr): mixed {
  return $obj->setAttributeNodeNS($newattr);
}

function dom_element_get_elements_by_tag_name_ns(
  DOMElement $obj,
  string $namespaceuri,
  string $localname,
): DOMNodeList {
  return $obj->getElementsByTagNameNS($namespaceuri, $localname);
}

function dom_element_has_attribute(DOMElement $obj, string $name): bool {
  return $obj->hasAttribute($name);
}

function dom_element_has_attribute_ns(DOMElement $obj,
                                      string $namespaceuri,
                                      string $localname): bool {
  return $obj->hasAttributeNS($namespaceuri, $localname);
}

function dom_element_set_id_attribute(DOMElement $obj,
                                      string $name,
                                      bool $isid): mixed {
  return $obj->setIDAttribute($name, $isid);
}

function dom_element_set_id_attribute_ns(DOMElement $obj,
                                         string $namespaceuri,
                                         string $localname,
                                         bool $isid): mixed {
  return $obj->setIDAttributeNS($namespaceuri, $localname, $isid);
}

function dom_element_set_id_attribute_node(DOMElement $obj,
                                           DOMAttr $idattr,
                                           bool $isid): mixed {
  return $obj->setIDAttributeNode($idattr, $isid);
}

function dom_text_split_text(DOMText $obj, int $offset): mixed {
  return $obj->splitText($offset);
}

function dom_text_is_whitespace_in_element_content(DOMText $obj): bool {
  return $obj->isWhitespaceInElementContent();
}

function dom_xpath_register_ns(DOMXPath $obj,
                               string $prefix,
                               string $uri): mixed {
  return $obj->registerNamespace($prefix, $uri);
}

/**
 * Executes the given XPath expression.
 *
 * @param DOMXPath $obj
 * @param string $expr - The XPath expression to execute.
 * @param DOMNode $context - The optional contextnode can be specified for
 *   doing relative XPath queries. By default, the queries are relative to the
 *   root element.
 *
 * @return mixed - Returns a DOMNodeList containing all nodes matching the
 *   given XPath expression. Any expression which do not return nodes will
 *   return an empty DOMNodeList.
 *
 */
function dom_xpath_query(DOMXPath $obj,
                         string $expr,
                         ?DOMNode $context = null): mixed {
  return $obj->query($expr, $context);
}

/**
 * Executes the given XPath expression and returns a typed result if
 *   possible.
 *
 * @param DOMXPath $obj
 * @param string $expr - The XPath expression to execute.
 * @param DOMNode $context - The optional contextnode can be specified for
 *   doing relative XPath queries. By default, the queries are relative to the
 *   root element.
 *
 * @return mixed - Returns a typed result if possible or a DOMNodeList
 *   containing all nodes matching the given XPath expression.
 *
 */
function dom_xpath_evaluate(DOMXPath $obj,
                            string $expr,
                            ?DOMNode $context = null): mixed {
  return $obj->evaluate($expr, $context);
}

function dom_xpath_register_php_functions(DOMXPath $obj,
                                          mixed $funcs = null): mixed {
  return $obj->registerPHPFunctions($funcs);
}

/**
 * This function takes the node node of class SimpleXML and makes it into a
 *   DOMElement node. This new object can then be used as a native DOMElement
 *   node.
 *
 * @param SimpleXMLElement $node - The SimpleXMLElement node.
 *
 * @return mixed - The DOMElement node added or FALSE if any errors occur.
 *
 */
<<__Native>>
function dom_import_simplexml(SimpleXMLElement $node): mixed;
