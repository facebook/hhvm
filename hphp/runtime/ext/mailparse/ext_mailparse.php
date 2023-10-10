<?hh
/* Create a MIME mail resource.
 * @return resource - Returns a handle that can be used to parse a message.
 */
<<__Native>>
function mailparse_msg_create(): resource;

/* Frees a MIME resource.
 * @param resource $mimemail - A valid MIME resource allocated by
 * mailparse_msg_create() or mailparse_msg_parse_file().
 * @return bool - Returns TRUE on success or FALSE on failure.
 */
<<__Native>>
function mailparse_msg_free(resource $mimemail): bool;

/* Parses a file. This is the optimal way of parsing a mail file that you have
 * on disk.
 * @param string $filename - Path to the file holding the message. The file is
 * opened and streamed through the parser.
 * @return mixed - Returns a MIME resource representing the structure, or
 * FALSE on error.
 */
<<__Native>>
function mailparse_msg_parse_file(string $filename): mixed;

/* Incrementally parse data into the supplied mime mail resource.  This
 * function allow you to stream portions of a file at a time, rather than read
 * and parse the whole thing.
 * @param resource $mimemail - A valid MIME resource.
 * @param string $data
 * @return bool - Returns TRUE on success or FALSE on failure.
 */
<<__Native>>
function mailparse_msg_parse(resource $mimemail,
                             string $data): bool;

/* Extracts/decodes a message section from the supplied filename.  The
 * contents of the section will be decoded according to their transfer
 * encoding - base64, quoted-printable and uuencoded text are supported.
 * @param resource $mimemail - A valid MIME resource, created with
 * mailparse_msg_create().
 * @param mixed $filename - Can be a file name or a valid stream resource.
 * @param mixed $callbackfunc - If set, this must be either a valid callback
 * that will be passed the extracted section, or NULL to make this function
 * return the extracted section.  If not specified, the contents will be sent
 * to "stdout".
 * @return mixed - If callbackfunc is not NULL returns TRUE on success.  If
 * callbackfunc is set to NULL, returns the extracted section as a string.
 * Returns FALSE on error.
 */
<<__Native>>
function mailparse_msg_extract_part_file(resource $mimemail,
                                         mixed $filename,
                                         mixed $callbackfunc = ""): mixed;

/* @param resource $mimemail - A valid MIME resource.
 * @param mixed $filename
 * @param mixed $callbackfunc
 * @return mixed
 */
<<__Native>>
function mailparse_msg_extract_whole_part_file(resource $mimemail,
                                               mixed $filename,
                                               mixed $callbackfunc = ""): mixed;

/* @param resource $mimemail - A valid MIME resource.
 * @param mixed $msgbody
 * @param mixed $callbackfunc
 * @return mixed - No value is returned.
 */
<<__Native>>
function mailparse_msg_extract_part(resource $mimemail,
                                    mixed $msgbody,
                                    mixed $callbackfunc = ""): mixed;

/* @param resource $mimemail
 * @return array
 */
<<__Native>>
function mailparse_msg_get_part_data(resource $mimemail): shape(
  'headers' => darray<arraykey, mixed>,
  "starting-pos" => int,
  "starting-pos-body" => int,
  "ending-pos" => int,
  "ending-pos-body" => int,
  "line-count" => int,
  "body-line-count" => int,
  "charset" => string,
  "transfer-encoding" => string,
  "content-type" => mixed,
  ?"content-disposition" => mixed,
  ?"content-location" => string,
  "content-base" => string,
  ?"content-boundary" => string,
  ?"content-id" => string,
  ?"content-description" => mixed,
  ?"content-language" => mixed,
  ?"content-md5" => mixed,
  ...
);

/* @param resource $mimemail
 * @param string $mimesection
 * @return mixed
 */
<<__Native>>
function mailparse_msg_get_part(resource $mimemail,
                                string $mimesection): mixed;

/* @param resource $mimemail
 * @return array
 */
<<__Native>>
function mailparse_msg_get_structure(resource $mimemail): darray<arraykey, string>;

/* @param string $addresses - A string containing addresses, like in: Wez
 * Furlong <wez@example.com>, doe@example.com  This string must not include
 * the header name.
 * @return array - Returns an array of associative arrays with the following
 * keys for each recipient: display The recipient name, for display purpose.
 * If this part is not set for a recipient, this key will hold the same value
 * as address. address The email address is_group TRUE if the recipient is a
 * newsgroup, FALSE otherwise.
 */
<<__Native>>
function mailparse_rfc822_parse_addresses(string $addresses): varray<shape(
  ?'display' => string,
  ?'address' => string,
  'is_group' => bool,
)>;

/* Streams data from the source file pointer, apply encoding and write to the
 * destination file pointer.
 * @param resource $sourcefp - A valid file handle. The file is streamed
 * through the parser.
 * @param resource $destfp - The destination file handle in which the encoded
 * data will be written.
 * @param string $encoding - One of the character encodings supported by the
 * mbstring module.
 * @return bool - Returns TRUE on success or FALSE on failure.
 */
<<__Native>>
function mailparse_stream_encode(resource $sourcefp,
                                 resource $destfp,
                                 string $encoding): bool;

/* Scans the data from the given file pointer and extract each embedded
 * uuencoded file into a temporary file.
 * @param resource $fp - A valid file pointer.
 * @return mixed - Returns an array of associative arrays listing filename
 * information. filename Path to the temporary file name created origfilename
 * The original filename, for uuencoded parts only The first filename entry is
 * the message body. The next entries are the decoded uuencoded files.
 */
<<__Native>>
function mailparse_uudecode_all(resource $fp): mixed;

/* Figures out the best way of encoding the content read from the given file
 * pointer.
 * @param resource $fp - A valid file pointer, which must be seek-able.
 * @return mixed - Returns one of the character encodings supported by the
 * mbstring module.
 */
<<__Native>>
function mailparse_determine_best_xfer_encoding(resource $fp): mixed;
