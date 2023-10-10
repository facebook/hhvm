<?hh

/**
 * xml_parser_create() creates a new XML parser and returns a resource handle
 *   referencing it to be used by the other XML functions.
 *
 * @param string $encoding - The optional encoding specifies the character
 *   encoding for the input/output in PHP 4. Starting from PHP 5, the input
 *   encoding is automatically detected, so that the encoding parameter
 *   specifies only the output encoding. In PHP 4, the default output encoding
 *   is the same as the input charset. If empty string is passed, the parser
 *   attempts to identify which encoding the document is encoded in by looking
 *   at the heading 3 or 4 bytes. In PHP 5.0.0 and 5.0.1, the default output
 *   charset is ISO-8859-1, while in PHP 5.0.2 and upper is UTF-8. The supported
 *   encodings are ISO-8859-1, UTF-8 and US-ASCII.
 *
 * @return resource - Returns a resource handle for the new XML parser.
 *
 */
<<__Native>>
function xml_parser_create(?string $encoding = null): resource;

/**
 * Frees the given XML parser.
 *
 * @param resource $parser - A reference to the XML parser to free.
 *
 * @return bool - This function returns FALSE if parser does not refer to a
 *   valid parser, or else it frees the parser and returns TRUE.
 *
 */
<<__Native>>
function xml_parser_free(resource $parser): bool;

/**
 * xml_parse() parses an XML document. The handlers for the configured events
 *   are called as many times as necessary.
 *
 * @param resource $parser - A reference to the XML parser to use.
 * @param string $data - Chunk of data to parse. A document may be parsed
 *   piece-wise by calling xml_parse() several times with new data, as long as
 *   the is_final parameter is set and TRUE when the last data is parsed.
 * @param bool $is_final - If set and TRUE, data is the last piece of data
 *   sent in this parse.
 *
 * @return int - Returns 1 on success or 0 on failure.  For unsuccessful
 *   parses, error information can be retrieved with xml_get_error_code(),
 *   xml_error_string(), xml_get_current_line_number(),
 *   xml_get_current_column_number() and xml_get_current_byte_index().  Entity
 *   errors are reported at the end of the data thus only if is_final is set and
 *   TRUE.
 *
 */
<<__Native>>
function xml_parse(resource $parser, string $data, bool $is_final = true): int;

/**
 * This function parses an XML file into 2 parallel array structures, one
 *   (index) containing pointers to the location of the appropriate values in
 *   the values array. These last two parameters must be passed by reference.
 *
 * @return int - xml_parse_into_struct() returns 0 for failure and 1 for
 *   success. This is not the same as FALSE and TRUE, be careful with operators
 *   such as ===.
 *
 */
<<__Native>>
function xml_parse_into_struct(resource $parser,
                               string $data,
                               <<__OutOnly("varray")>>
                               inout mixed $values,
                               <<__OutOnly("darray")>>
                               inout mixed $index): int;

/**
 * xml_parser_create_ns() creates a new XML parser with XML namespace support
 *   and returns a resource handle referencing it to be used by the other XML
 *   functions.
 *
 * @param string $encoding - The optional encoding specifies the character
 *   encoding for the input/output in PHP 4. Starting from PHP 5, the input
 *   encoding is automatically detected, so that the encoding parameter
 *   specifies only the output encoding. In PHP 4, the default output encoding
 *   is the same as the input charset. In PHP 5.0.0 and 5.0.1, the default
 *   output charset is ISO-8859-1, while in PHP 5.0.2 and upper is UTF-8. The
 *   supported encodings are ISO-8859-1, UTF-8 and US-ASCII.
 * @param string $separator - With a namespace aware parser tag parameters
 *   passed to the various handler functions will consist of namespace and tag
 *   name separated by the string specified in separator.
 *
 * @return resource - Returns a resource handle for the new XML parser.
 *
 */
<<__Native>>
function xml_parser_create_ns(?string $encoding = null,
                              ?string $separator = null): resource;

/**
 * Gets an option value from an XML parser.
 *
 * @param resource $parser - A reference to the XML parser to get an option
 *   from.
 * @param int $option - Which option to fetch. XML_OPTION_CASE_FOLDING and
 *   XML_OPTION_TARGET_ENCODING are available. See xml_parser_set_option() for
 *   their description.
 *
 * @return mixed - This function returns FALSE if parser does not refer to a
 *   valid parser or if option isn't valid (generates also a E_WARNING). Else
 *   the option's value is returned.
 *
 */
<<__Native>>
function xml_parser_get_option(resource $parser, int $option): mixed;

/**
 * Sets an option in an XML parser.
 *
 * @param resource $parser - A reference to the XML parser to set an option
 *   in.
 * @param int $option - Which option to set. See below.  The following options
 *   are available: XML parser options Option constant Data type Description
 *   XML_OPTION_CASE_FOLDING integer Controls whether case-folding is enabled
 *   for this XML parser. Enabled by default. XML_OPTION_SKIP_TAGSTART integer
 *   Specify how many characters should be skipped in the beginning of a tag
 *   name. XML_OPTION_SKIP_WHITE integer Whether to skip values consisting of
 *   whitespace characters. XML_OPTION_TARGET_ENCODING string Sets which target
 *   encoding to use in this XML parser.By default, it is set to the same as the
 *   source encoding used by xml_parser_create(). Supported target encodings are
 *   ISO-8859-1, US-ASCII and UTF-8.
 * @param mixed $value - The option's new value.
 *
 * @return bool - This function returns FALSE if parser does not refer to a
 *   valid parser, or if the option could not be set. Else the option is set and
 *   TRUE is returned.
 *
 */
<<__Native>>
function xml_parser_set_option(resource $parser,
                               int $option,
                               mixed $value): bool;

/**
 * Sets the character data handler function for the XML parser parser.
 *
 * @param resource $parser
 * @param mixed $handler - handler is a string containing the name of a
 *   function that must exist when xml_parse() is called for parser.  The
 *   function named by handler must accept two parameters: handler ( resource
 *   $parser , string $data ) parser The first parameter, parser, is a reference
 *   to the XML parser calling the handler.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function xml_set_character_data_handler(resource $parser, mixed $handler): bool;

/**
 * Sets the default handler function for the XML parser parser.
 *
 * @param resource $parser
 * @param mixed $handler - handler is a string containing the name of a
 *   function that must exist when xml_parse() is called for parser.  The
 *   function named by handler must accept two parameters: handler ( resource
 *   $parser , string $data ) parser The first parameter, parser, is a reference
 *   to the XML parser calling the handler.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function xml_set_default_handler(resource $parser, mixed $handler): bool;

/**
 * Sets the element handler functions for the XML parser.
 *   start_element_handler and end_element_handler are strings containing the
 *   names of functions that must exist when xml_parse() is called for parser.
 *
 * @param resource $parser
 * @param mixed $start_element_handler - The function named by
 *   start_element_handler must accept three parameters: start_element_handler (
 *   resource $parser , string $name , array $attribs ) parser The first
 *   parameter, parser, is a reference to the XML parser calling the handler.
 * @param mixed $end_element_handler - The second parameter, name, contains
 *   the name of the element for which this handler is called.If case-folding is
 *   in effect for this parser, the element name will be in uppercase letters.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function xml_set_element_handler(resource $parser,
                                 mixed $start_element_handler,
                                 mixed $end_element_handler): bool;

/**
 * Sets the processing instruction (PI) handler function for the XML parser
 *   parser.  A processing instruction has the following format: target data?>
 *   You can put PHP code into such a tag, but be aware of one limitation: in an
 *   XML PI, the PI end tag (?>) can not be quoted, so this character sequence
 *   should not appear in the PHP code you embed with PIs in XML documents.If it
 *   does, the rest of the PHP code, as well as the "real" PI end tag, will be
 *   treated as character data.
 *
 * @param resource $parser
 * @param mixed $handler - handler is a string containing the name of a
 *   function that must exist when xml_parse() is called for parser.  The
 *   function named by handler must accept three parameters: handler ( resource
 *   $parser , string $target , string $data ) parser The first parameter,
 *   parser, is a reference to the XML parser calling the handler.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function xml_set_processing_instruction_handler(resource $parser,
                                                mixed $handler): bool;

/**
 * Set a handler to be called when a namespace is declared. Namespace
 *   declarations occur inside start tags. But the namespace declaration start
 *   handler is called before the start tag handler for each namespace declared
 *   in that start tag.
 *
 * @param resource $parser - A reference to the XML parser.
 * @param mixed $handler - handler is a string containing the name of a
 *   function that must exist when xml_parse() is called for parser.  The
 *   function named by handler must accept four parameters, and should return an
 *   integer value. If the value returned from the handler is FALSE (which it
 *   will be if no value is returned), the XML parser will stop parsing and
 *   xml_get_error_code() will return XML_ERROR_EXTERNAL_ENTITY_HANDLING.
 *   handler ( resource $parser , string $user_data , string $prefix , string
 *   $uri ) parser The first parameter, parser, is a reference to the XML parser
 *   calling the handler.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function xml_set_start_namespace_decl_handler(resource $parser,
                                              mixed $handler): bool;

/**
 * Set a handler to be called when leaving the scope of a namespace
 *   declaration. This will be called, for each namespace declaration, after the
 *   handler for the end tag of the element in which the namespace was declared.
 *
 * @param resource $parser - A reference to the XML parser.
 * @param mixed $handler - handler is a string containing the name of a
 *   function that must exist when xml_parse() is called for parser.  The
 *   function named by handler must accept three parameters, and should return
 *   an integer value. If the value returned from the handler is FALSE (which it
 *   will be if no value is returned), the XML parser will stop parsing and
 *   xml_get_error_code() will return XML_ERROR_EXTERNAL_ENTITY_HANDLING.
 *   handler ( resource $parser , string $user_data , string $prefix ) parser
 *   The first parameter, parser, is a reference to the XML parser calling the
 *   handler.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function xml_set_end_namespace_decl_handler(resource $parser,
                                            mixed $handler): bool;

/**
 * @param resource $parser
 * @param mixed $handler - handler is a string containing the name of a
 *   function that must exist when xml_parse() is called for parser.  The
 *   function named by handler must accept six parameters: handler ( resource
 *   $parser , string $entity_name , string $base , string $system_id , string
 *   $public_id , string $notation_name ) parser The first parameter, parser, is
 *   a reference to the XML parser calling the handler.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function xml_set_unparsed_entity_decl_handler(resource $parser,
                                              mixed $handler): bool;

/**
 * Sets the external entity reference handler function for the XML parser
 *   parser.
 *
 * @param resource $parser
 * @param mixed $handler - handler is a string containing the name of a
 *   function that must exist when xml_parse() is called for parser.  The
 *   function named by handler must accept five parameters, and should return an
 *   integer value.If the value returned from the handler is FALSE (which it
 *   will be if no value is returned), the XML parser will stop parsing and
 *   xml_get_error_code() will return XML_ERROR_EXTERNAL_ENTITY_HANDLING.
 *   handler ( resource $parser , string $open_entity_names , string $base ,
 *   string $system_id , string $public_id ) parser The first parameter, parser,
 *   is a reference to the XML parser calling the handler.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function xml_set_external_entity_ref_handler(resource $parser,
                                             mixed $handler): bool;

/**
 * @param resource $parser
 * @param mixed $handler - handler is a string containing the name of a
 *   function that must exist when xml_parse() is called for parser.  The
 *   function named by handler must accept five parameters: handler ( resource
 *   $parser , string $notation_name , string $base , string $system_id , string
 *   $public_id ) parser The first parameter, parser, is a reference to the XML
 *   parser calling the handler.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function xml_set_notation_decl_handler(resource $parser, mixed $handler): bool;

/**
 * This function allows to use parser inside object. All callback functions
 *   could be set with xml_set_element_handler() etc and assumed to be methods
 *   of object.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function xml_set_object(resource $parser, mixed $object): bool;

/**
 * Gets the current byte index of the given XML parser.
 *
 * @param resource $parser - A reference to the XML parser to get byte index
 *   from.
 *
 * @return int - This function returns FALSE if parser does not refer to a
 *   valid parser, or else it returns which byte index the parser is currently
 *   at in its data buffer (starting at 0).
 *
 */
<<__Native>>
function xml_get_current_byte_index(resource $parser): int;

/**
 * Gets the current column number of the given XML parser.
 *
 * @param resource $parser - A reference to the XML parser to get column
 *   number from.
 *
 * @return int - This function returns FALSE if parser does not refer to a
 *   valid parser, or else it returns which column on the current line (as given
 *   by xml_get_current_line_number()) the parser is currently at.
 *
 */
<<__Native>>
function xml_get_current_column_number(resource $parser): int;

/**
 * Gets the current line number for the given XML parser.
 *
 * @param resource $parser - A reference to the XML parser to get line number
 *   from.
 *
 * @return int - This function returns FALSE if parser does not refer to a
 *   valid parser, or else it returns which line the parser is currently at in
 *   its data buffer.
 *
 */
<<__Native>>
function xml_get_current_line_number(resource $parser): int;

/**
 * Gets the XML parser error code.
 *
 * @param resource $parser - A reference to the XML parser to get error code
 *   from.
 *
 * @return int - This function returns FALSE if parser does not refer to a
 *   valid parser, or else it returns one of the error codes listed in the error
 *   codes section.
 *
 */
<<__Native>>
function xml_get_error_code(resource $parser): int;

/**
 * Gets the XML parser error string associated with the given code.
 *
 * @param int $code - An error code from xml_get_error_code().
 *
 * @return string - Returns a string with a textual description of the error
 *   code, or FALSE if no description was found.
 *
 */
<<__Native>>
function xml_error_string(int $code): string;

/**
 * This function decodes data, assumed to be UTF-8 encoded, to ISO-8859-1.
 *
 * @param string $data - An UTF-8 encoded string.
 *
 * @return string - Returns the ISO-8859-1 translation of data.
 *
 */
<<__Native, __IsFoldable>>
function utf8_decode(string $data)[]: string;

/**
 * This function encodes the string data to UTF-8, and returns the encoded
 *   version. UTF-8 is a standard mechanism used by Unicode for encoding wide
 *   character values into a byte stream. UTF-8 is transparent to plain ASCII
 *   characters, is self-synchronized (meaning it is possible for a program to
 *   figure out where in the bytestream characters start) and can be used with
 *   normal string comparison functions for sorting and such. PHP encodes UTF-8
 *   characters in up to four bytes, like this: UTF-8 encoding bytes bits
 *   representation 1 7 0bbbbbbb 2 11 110bbbbb 10bbbbbb 3 16 1110bbbb 10bbbbbb
 *   10bbbbbb 4 21 11110bbb 10bbbbbb 10bbbbbb 10bbbbbb Each b represents a bit
 *   that can be used to store character data.
 *
 * @param string $data - An ISO-8859-1 string.
 *
 * @return string - Returns the UTF-8 translation of data.
 *
 */
<<__Native, __IsFoldable>>
function utf8_encode(string $data)[]: string;
