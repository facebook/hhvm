<?hh

/**
 * Establishes a connection to a LDAP server on a specified hostname and port.
 *
 * @param string $hostname - If you are using OpenLDAP 2.x.x you can specify a
 *   URL instead of the hostname. To use LDAP with SSL, compile OpenLDAP 2.x.x
 *   with SSL support, configure PHP with SSL, and set this parameter as
 *   ldaps://hostname/.
 * @param int $port - The port to connect to. Not used when using URLs.
 *
 * @return mixed - Returns a positive LDAP link identifier on success, or
 *   FALSE on error. When OpenLDAP 2.x.x is used, ldap_connect() will always
 *   return a resource as it does not actually connect but just initializes the
 *   connecting parameters. The actual connect happens with the next calls to
 *   ldap_* funcs, usually with ldap_bind().  If no arguments are specified then
 *   the link identifier of the already opened link will be returned.
 *
 */
<<__Native>>
function ldap_connect(?string $hostname = null, int $port = 389): mixed;

/**
 * Splits the DN returned by ldap_get_dn() and breaks it up into its component
 *   parts. Each part is known as Relative Distinguished Name, or RDN.
 *
 * @param string $dn - The distinguished name of an LDAP entity.
 * @param int $with_attrib - Used to request if the RDNs are returned with
 *   only values or their attributes as well. To get RDNs with the attributes
 *   (i.e. in attribute=value format) set with_attrib to 0 and to get only
 *   values set it to 1.
 *
 * @return mixed - Returns an array of all DN components. The first element in
 *   this array has count key and represents the number of returned values, next
 *   elements are numerically indexed DN components.
 *
 */
<<__Native>>
function ldap_explode_dn(string $dn, int $with_attrib): mixed;

/**
 * Turns the specified dn, into a more user-friendly form, stripping off type
 *   names.
 *
 * @param string $db - The distinguished name of an LDAP entity.
 *
 * @return mixed - Returns the user friendly name.
 *
 */
<<__Native>>
function ldap_dn2ufn(string $db): mixed;

/**
 * Returns the string error message explaining the error number errno. While
 *   LDAP errno numbers are standardized, different libraries return different
 *   or even localized textual error messages. Never check for a specific error
 *   message text, but always use an error number to check.
 *
 * @param int $errnum - The error number.
 *
 * @return string - Returns the error message, as a string.
 *
 */
<<__Native>>
function ldap_err2str(int $errnum): string;

/**
 * Add entries in the LDAP directory.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param string $dn - The distinguished name of an LDAP entity.
 * @param array $entry - An array that specifies the information about the
 *   entry. The values in the entries are indexed by individual attributes. In
 *   case of multiple values for an attribute, they are indexed using integers
 *   starting with 0.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function ldap_add(resource $link, string $dn, darray $entry): bool;

/**
 * Adds one or more attributes to the specified dn. It performs the
 *   modification at the attribute level as opposed to the object level.
 *   Object-level additions are done by the ldap_add() function.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param string $dn - The distinguished name of an LDAP entity.
 * @param array $entry
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function ldap_mod_add(resource $link, string $dn, darray $entry): bool;

/**
 * Removes one or more attributes from the specified dn. It performs the
 *   modification at the attribute level as opposed to the object level.
 *   Object-level deletions are done by the ldap_delete() function.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param string $dn - The distinguished name of an LDAP entity.
 * @param array $entry
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function ldap_mod_del(resource $link, string $dn, darray $entry): bool;

/**
 * Replaces one or more attributes from the specified dn. It performs the
 *   modification at the attribute level as opposed to the object level.
 *   Object-level modifications are done by the ldap_modify() function.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param string $dn - The distinguished name of an LDAP entity.
 * @param array $entry
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function ldap_mod_replace(resource $link, string $dn, darray $entry): bool;

/**
 * Modify the existing entries in the LDAP directory. The structure of the
 *   entry is same as in ldap_add().
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param string $dn - The distinguished name of an LDAP entity.
 * @param array $entry
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function ldap_modify(resource $link, string $dn, darray $entry): bool;

/**
 * Modify the existing entries in the LDAP directory. Allows detailed
 *   specification of the modifications to perform.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param string $dn - The distinguished name of an LDAP entity.
 * @param array $modifs - An array specifying the modifications to perform.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 */
<<__Native>>
function ldap_modify_batch(resource $link, string $dn, varray $modifs): bool;

/**
 * Binds to the LDAP directory with specified RDN and password. If bind_rdn
 *   and bind_password are not specified, an anonymous bind is attempted.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param string $bind_rdn
 * @param string $bind_password
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function ldap_bind(resource $link,
                   ?string $bind_rdn = null,
                   ?string $bind_password = null): bool;

<<__Native>>
function ldap_set_rebind_proc(resource $link, mixed $callback): bool;

<<__Native>>
function ldap_sort(resource $link, resource $result, string $sortfilter): bool;

<<__Native>>
function ldap_start_tls(resource $link): bool;

/**
 * Unbinds from the LDAP directory.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function ldap_unbind(resource $link): bool;

/**
 * Sets retval to the value of the specified option.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param int $option - The parameter option can be one of: Option Type
 *   LDAP_OPT_DEREF integer LDAP_OPT_SIZELIMIT integer LDAP_OPT_TIMELIMIT
 *   integer LDAP_OPT_NETWORK_TIMEOUT integer LDAP_OPT_PROTOCOL_VERSION integer
 *   LDAP_OPT_ERROR_NUMBER integer LDAP_OPT_REFERRALS bool LDAP_OPT_RESTART bool
 *   LDAP_OPT_HOST_NAME string LDAP_OPT_ERROR_STRING string LDAP_OPT_MATCHED_DN
 *   string LDAP_OPT_SERVER_CONTROLS array LDAP_OPT_CLIENT_CONTROLS array
 * @param mixed $retval - This will be set to the option value.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function ldap_get_option(resource $link,
                         int $option,
                         <<__OutOnly>>
                         inout mixed $retval): bool;

/**
 * Sets the value of the specified option to be newval.
 *
 * @param mixed $link - An LDAP link identifier, returned by ldap_connect().
 * @param int $option
 * @param mixed $newval - The new value for the specified option.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function ldap_set_option(mixed $link, int $option, mixed $newval): bool;

<<__Native>>
function ldap_close(resource $link): bool;

/**
 * Performs the search for a specified filter on the directory with the scope
 *   LDAP_SCOPE_ONELEVEL.  LDAP_SCOPE_ONELEVEL means that the search should only
 *   return information that is at the level immediately below the base_dn given
 *   in the call. (Equivalent to typing "ls" and getting a list of files and
 *   folders in the current working directory.)
 *
 * @param mixed $link - An LDAP link identifier, returned by ldap_connect().
 * @param mixed $base_dn - The base DN for the directory.
 * @param mixed $filter
 * @param array $attributes - An array of the required attributes, e.g.
 *   array("mail", "sn", "cn"). Note that the "dn" is always returned
 *   irrespective of which attributes types are requested.  Using this parameter
 *   is much more efficient than the default action (which is to return all
 *   attributes and their associated values). The use of this parameter should
 *   therefore be considered good practice.
 * @param int $attrsonly - Should be set to 1 if only attribute types are
 *   wanted. If set to 0 both attributes types and attribute values are fetched
 *   which is the default behaviour.
 * @param int $sizelimit - Enables you to limit the count of entries fetched.
 *   Setting this to 0 means no limit.  This parameter can NOT override
 *   server-side preset sizelimit. You can set it lower though.  Some directory
 *   server hosts will be configured to return no more than a preset number of
 *   entries. If this occurs, the server will indicate that it has only returned
 *   a partial results set. This also occurs if you use this parameter to limit
 *   the count of fetched entries.
 * @param int $timelimit - Sets the number of seconds how long is spend on the
 *   search. Setting this to 0 means no limit.  This parameter can NOT override
 *   server-side preset timelimit. You can set it lower though.
 * @param int $deref - Specifies how aliases should be handled during the
 *   search. It can be one of the following: LDAP_DEREF_NEVER - (default)
 *   aliases are never dereferenced. LDAP_DEREF_SEARCHING - aliases should be
 *   dereferenced during the search but not when locating the base object of the
 *   search. LDAP_DEREF_FINDING - aliases should be dereferenced when locating
 *   the base object but not during the search. LDAP_DEREF_ALWAYS - aliases
 *   should be dereferenced always.
 *
 * @return mixed - Returns a search result identifier or FALSE on error.
 *
 */
<<__Native>>
function ldap_list(mixed $link,
                   mixed $base_dn,
                   mixed $filter,
                   ?varray $attributes = null,
                   int $attrsonly = 0,
                   int $sizelimit = -1,
                   int $timelimit = -1,
                   int $deref = -1): mixed;

/**
 * Performs the search for a specified filter on the directory with the scope
 *   LDAP_SCOPE_BASE. So it is equivalent to reading an entry from the
 *   directory.
 *
 * @param mixed $link - An LDAP link identifier, returned by ldap_connect().
 * @param mixed $base_dn - The base DN for the directory.
 * @param mixed $filter - An empty filter is not allowed. If you want to
 *   retrieve absolutely all information for this entry, use a filter of
 *   objectClass=*. If you know which entry types are used on the directory
 *   server, you might use an appropriate filter such as
 *   objectClass=inetOrgPerson.
 * @param array $attributes - An array of the required attributes, e.g.
 *   array("mail", "sn", "cn"). Note that the "dn" is always returned
 *   irrespective of which attributes types are requested.  Using this parameter
 *   is much more efficient than the default action (which is to return all
 *   attributes and their associated values). The use of this parameter should
 *   therefore be considered good practice.
 * @param int $attrsonly - Should be set to 1 if only attribute types are
 *   wanted. If set to 0 both attributes types and attribute values are fetched
 *   which is the default behaviour.
 * @param int $sizelimit - Enables you to limit the count of entries fetched.
 *   Setting this to 0 means no limit.  This parameter can NOT override
 *   server-side preset sizelimit. You can set it lower though.  Some directory
 *   server hosts will be configured to return no more than a preset number of
 *   entries. If this occurs, the server will indicate that it has only returned
 *   a partial results set. This also occurs if you use this parameter to limit
 *   the count of fetched entries.
 * @param int $timelimit - Sets the number of seconds how long is spend on the
 *   search. Setting this to 0 means no limit.  This parameter can NOT override
 *   server-side preset timelimit. You can set it lower though.
 * @param int $deref - Specifies how aliases should be handled during the
 *   search. It can be one of the following: LDAP_DEREF_NEVER - (default)
 *   aliases are never dereferenced. LDAP_DEREF_SEARCHING - aliases should be
 *   dereferenced during the search but not when locating the base object of the
 *   search. LDAP_DEREF_FINDING - aliases should be dereferenced when locating
 *   the base object but not during the search. LDAP_DEREF_ALWAYS - aliases
 *   should be dereferenced always.
 *
 * @return mixed - Returns a search result identifier or FALSE on error.
 *
 */
<<__Native>>
function ldap_read(mixed $link,
                   mixed $base_dn,
                   mixed $filter,
                   ?varray $attributes = null,
                   int $attrsonly = 0,
                   int $sizelimit = -1,
                   int $timelimit = -1,
                   int $deref = -1): mixed;

/**
 * Performs the search for a specified filter on the directory with the scope
 *   of LDAP_SCOPE_SUBTREE. This is equivalent to searching the entire
 *   directory.  From 4.0.5 on it's also possible to do parallel searches. To do
 *   this you use an array of link identifiers, rather than a single identifier,
 *   as the first argument. If you don't want the same base DN and the same
 *   filter for all the searches, you can also use an array of base DNs and/or
 *   an array of filters. Those arrays must be of the same size as the link
 *   identifier array since the first entries of the arrays are used for one
 *   search, the second entries are used for another, and so on. When doing
 *   parallel searches an array of search result identifiers is returned, except
 *   in case of error, then the entry corresponding to the search will be FALSE.
 *   This is very much like the value normally returned, except that a result
 *   identifier is always returned when a search was made. There are some rare
 *   cases where the normal search returns FALSE while the parallel search
 *   returns an identifier.
 *
 * @param mixed $link - An LDAP link identifier, returned by ldap_connect().
 * @param mixed $base_dn - The base DN for the directory.
 * @param mixed $filter
 * @param array $attributes - An array of the required attributes, e.g.
 *   array("mail", "sn", "cn"). Note that the "dn" is always returned
 *   irrespective of which attributes types are requested.  Using this parameter
 *   is much more efficient than the default action (which is to return all
 *   attributes and their associated values). The use of this parameter should
 *   therefore be considered good practice.
 * @param int $attrsonly - Should be set to 1 if only attribute types are
 *   wanted. If set to 0 both attributes types and attribute values are fetched
 *   which is the default behaviour.
 * @param int $sizelimit - Enables you to limit the count of entries fetched.
 *   Setting this to 0 means no limit.  This parameter can NOT override
 *   server-side preset sizelimit. You can set it lower though.  Some directory
 *   server hosts will be configured to return no more than a preset number of
 *   entries. If this occurs, the server will indicate that it has only returned
 *   a partial results set. This also occurs if you use this parameter to limit
 *   the count of fetched entries.
 * @param int $timelimit - Sets the number of seconds how long is spend on the
 *   search. Setting this to 0 means no limit.  This parameter can NOT override
 *   server-side preset timelimit. You can set it lower though.
 * @param int $deref - Specifies how aliases should be handled during the
 *   search. It can be one of the following: LDAP_DEREF_NEVER - (default)
 *   aliases are never dereferenced. LDAP_DEREF_SEARCHING - aliases should be
 *   dereferenced during the search but not when locating the base object of the
 *   search. LDAP_DEREF_FINDING - aliases should be dereferenced when locating
 *   the base object but not during the search. LDAP_DEREF_ALWAYS - aliases
 *   should be dereferenced always.
 *
 * @return mixed - Returns a search result identifier or FALSE on error.
 *
 */
<<__Native>>
function ldap_search(mixed $link,
                     mixed $base_dn,
                     mixed $filter,
                     ?varray $attributes = null,
                     int $attrsonly = 0,
                     int $sizelimit = -1,
                     int $timelimit = -1,
                     int $deref = -1): mixed;

/**
 * The entry specified by dn is renamed/moved.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param string $dn - The distinguished name of an LDAP entity.
 * @param string $newrdn - The new RDN.
 * @param string $newparent - The new parent/superior entry.
 * @param bool $deleteoldrdn - If TRUE the old RDN value(s) is removed, else
 *   the old RDN value(s) is retained as non-distinguished values of the entry.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function ldap_rename(resource $link,
                     string $dn,
                     string $newrdn,
                     string $newparent,
                     bool $deleteoldrdn): bool;

/**
 * Deletes a particular entry in LDAP directory.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param string $dn - The distinguished name of an LDAP entity.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function ldap_delete(resource $link, string $dn): bool;

/**
 * Compare value of attribute with value of same attribute in an LDAP
 *   directory entry.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param string $dn - The distinguished name of an LDAP entity.
 * @param string $attribute - The attribute name.
 * @param string $value - The compared value.
 *
 * @return mixed - Returns TRUE if value matches otherwise returns FALSE.
 *   Returns -1 on error.
 *
 */
<<__Native>>
function ldap_compare(resource $link,
                      string $dn,
                      string $attribute,
                      string $value): mixed;

/**
 * Returns the standardized error number returned by the last LDAP command.
 *   This number can be converted into a textual error message using
 *   ldap_err2str().
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 *
 * @return int - Return the LDAP error number of the last LDAP command for
 *   this link.
 *
 */
<<__Native>>
function ldap_errno(resource $link): mixed;

/**
 * Returns the string error message explaining the error generated by the last
 *   LDAP command for the given link_identifier. While LDAP errno numbers are
 *   standardized, different libraries return different or even localized
 *   textual error messages. Never check for a specific error message text, but
 *   always use an error number to check.  Unless you lower your warning level
 *   in your php.ini sufficiently or prefix your LDAP commands with @ (at)
 *   characters to suppress warning output, the errors generated will also show
 *   up in your HTML output.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 *
 * @return string - Returns string error message.
 *
 */
<<__Native>>
function ldap_error(resource $link): mixed;

/**
 * Finds out the DN of an entry in the result.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param resource $result_entry
 *
 * @return mixed - Returns the DN of the result entry and FALSE on error.
 *
 */
<<__Native>>
function ldap_get_dn(resource $link, resource $result_entry): mixed;

/**
 * Returns the number of entries stored in the result of previous search
 *   operations.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param resource $result - The internal LDAP result.
 *
 * @return int - Returns number of entries in the result or FALSE on error.
 *
 */
<<__Native>>
function ldap_count_entries(resource $link, resource $result): mixed;

/**
 * Reads multiple entries from the given result, and then reading the
 *   attributes and multiple values.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param resource $result
 *
 * @return mixed - Returns a complete result information in a
 *   multi-dimensional array on success and FALSE on error.  The structure of
 *   the array is as follows. The attribute index is converted to lowercase.
 *   (Attributes are case-insensitive for directory servers, but not when used
 *   as array indices.)
 *
 */
<<__Native>>
function ldap_get_entries(resource $link, resource $result): mixed;

/**
 * Returns the entry identifier for first entry in the result. This entry
 *   identifier is then supplied to ldap_next_entry() routine to get successive
 *   entries from the result.  Entries in the LDAP result are read sequentially
 *   using the ldap_first_entry() and ldap_next_entry() functions.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param resource $result
 *
 * @return mixed - Returns the result entry identifier for the first entry on
 *   success and FALSE on error.
 *
 */
<<__Native>>
function ldap_first_entry(resource $link, resource $result): mixed;

/**
 * Retrieve the entries stored in the result. Successive calls to the
 *   ldap_next_entry() return entries one by one till there are no more entries.
 *   The first call to ldap_next_entry() is made after the call to
 *   ldap_first_entry() with the result_entry_identifier as returned from the
 *   ldap_first_entry().
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param resource $result_entry
 *
 * @return mixed - Returns entry identifier for the next entry in the result
 *   whose entries are being read starting with ldap_first_entry(). If there are
 *   no more entries in the result then it returns FALSE.
 *
 */
<<__Native>>
function ldap_next_entry(resource $link, resource $result_entry): mixed;

/**
 * Reads attributes and values from an entry in the search result.  Having
 *   located a specific entry in the directory, you can find out what
 *   information is held for that entry by using this call. You would use this
 *   call for an application which "browses" directory entries and/or where you
 *   do not know the structure of the directory entries. In many applications
 *   you will be searching for a specific attribute such as an email address or
 *   a surname, and won't care what other data is held.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param resource $result_entry
 *
 * @return array - Returns a complete entry information in a multi-dimensional
 *   array on success and FALSE on error.
 *
 */
<<__Native>>
function ldap_get_attributes(resource $link, resource $result_entry): mixed;

/**
 * Gets the first attribute in the given entry. Remaining attributes are
 *   retrieved by calling ldap_next_attribute() successively.  Similar to
 *   reading entries, attributes are also read one by one from a particular
 *   entry.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param resource $result_entry
 *
 * @return mixed - Returns the first attribute in the entry on success and
 *   FALSE on error.
 *
 */
<<__Native>>
function ldap_first_attribute(resource $link, resource $result_entry): mixed;

/**
 * Retrieves the attributes in an entry. The first call to
 *   ldap_next_attribute() is made with the result_entry_identifier returned
 *   from ldap_first_attribute().
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param resource $result_entry
 *
 * @return mixed - Returns the next attribute in an entry on success and FALSE
 *   on error.
 *
 */
<<__Native>>
function ldap_next_attribute(resource $link, resource $result_entry): mixed;

<<__Native>>
function ldap_first_reference(resource $link, resource $result): mixed;

<<__Native>>
function ldap_next_reference(resource $link, resource $result_entry): mixed;

<<__Native>>
function ldap_parse_reference(resource $link,
                              resource $result_entry,
                              <<__OutOnly("varray")>>
                              inout mixed $referrals): bool;

<<__Native>>
function ldap_parse_result(resource $link,
                           resource $result,
                           <<__OutOnly("KindOfInt64")>>
                           inout mixed $errcode,
                           <<__OutOnly("KindOfString")>>
                           inout mixed $matcheddn,
                           <<__OutOnly("KindOfString")>>
                           inout mixed $errmsg,
                           <<__OutOnly("varray")>>
                           inout mixed $referrals): bool;

/**
 * Frees up the memory allocated internally to store the result. All result
 *   memory will be automatically freed when the script terminates.  Typically
 *   all the memory allocated for the LDAP result gets freed at the end of the
 *   script. In case the script is making successive searches which return large
 *   result sets, ldap_free_result() could be called to keep the runtime memory
 *   usage by the script low.
 *
 * @param resource $result
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function ldap_free_result(resource $result): bool;

/**
 * Reads all the values of the attribute in the entry in the result.  This
 *   function is used exactly like ldap_get_values() except that it handles
 *   binary data and not string data.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param resource $result_entry
 * @param string $attribute
 *
 * @return mixed - Returns an array of values for the attribute on success and
 *   FALSE on error. Individual values are accessed by integer index in the
 *   array. The first index is 0. The number of values can be found by indexing
 *   "count" in the resultant array.
 *
 */
<<__Native>>
function ldap_get_values_len(resource $link,
                             resource $result_entry,
                             string $attribute): mixed;

/**
 * Reads all the values of the attribute in the entry in the result.  This
 *   call needs a result_entry_identifier, so needs to be preceded by one of the
 *   ldap search calls and one of the calls to get an individual entry.  You
 *   application will either be hard coded to look for certain attributes (such
 *   as "surname" or "mail") or you will have to use the ldap_get_attributes()
 *   call to work out what attributes exist for a given entry.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param resource $result_entry
 * @param string $attribute
 *
 * @return mixed - Returns an array of values for the attribute on success and
 *   FALSE on error. The number of values can be found by indexing "count" in
 *   the resultant array. Individual values are accessed by integer index in the
 *   array. The first index is 0.  LDAP allows more than one entry for an
 *   attribute, so it can, for example, store a number of email addresses for
 *   one person's directory entry all labeled with the attribute "mail"
 *   return_value["count"] = number of values for attribute return_value[0] =
 *   first value of attribute return_value[i] = ith value of attribute
 *
 */
<<__Native>>
function ldap_get_values(resource $link,
                         resource $result_entry,
                         string $attribute): mixed;

/**
 * Enable LDAP pagination by sending the pagination control (page size,
 *   cookie...).
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param int $pagesize - The number of entries by page.
 * @param bool $iscritical - Indicates whether the pagination is critical of
 *   not. If true and if the server doesn't support pagination, the search will
 *   return no result.
 * @param string $cookie - An opaque structure sent by the server.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function ldap_control_paged_result(resource $link,
                                   int $pagesize,
                                   bool $iscritical = false,
                                   string $cookie = ""): bool;

/**
 * Retrieve the pagination information send by the server.
 *
 * @param resource $link - An LDAP link identifier, returned by
 *   ldap_connect().
 * @param resource $result
 * @param mixed $cookie - An opaque structure sent by the server.
 * @param mixed $estimated - The estimated number of entries to retrieve.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function ldap_control_paged_result_response(resource $link,
                                            resource $result,
                                            <<__OutOnly("KindOfString")>>
                                            inout mixed $cookie,
                                            <<__OutOnly("KindOfInt64")>>
                                            inout mixed $estimated): bool;

/**
 * Escape a string for use in an LDAP filter or DN.
 *
 * @param string $value - The value to escape.
 * @param string $ignore - Characters to ignore when escaping.
 * @param int $flags - Flags indicating the context the escaped string
 *   will be used in (combined with the | bitwise operator):
 *   integer LDAP_ESCAPE_FILTER for filters to be used with ldap_search(), or
 *   integer LDAP_ESCAPE_DN for DNs.
 *
 * @return string - Returns the escaped string.
 *
 */
<<__Native>>
function ldap_escape(string $value,
                     string $ignore = "",
                     int $flags = 0): string;
