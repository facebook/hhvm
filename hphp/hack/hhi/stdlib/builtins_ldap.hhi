<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function ldap_connect($hostname = null, int $port = 389);
<<__PHPStdLib>>
function ldap_explode_dn(string $dn, int $with_attrib);
<<__PHPStdLib>>
function ldap_dn2ufn(string $db);
<<__PHPStdLib>>
function ldap_err2str(int $errnum);
<<__PHPStdLib>>
function ldap_add(resource $link, string $dn, $entry);
<<__PHPStdLib>>
function ldap_mod_add(resource $link, string $dn, $entry);
<<__PHPStdLib>>
function ldap_mod_del(resource $link, string $dn, $entry);
<<__PHPStdLib>>
function ldap_mod_replace(resource $link, string $dn, $entry);
<<__PHPStdLib>>
function ldap_modify(resource $link, string $dn, $entry);
<<__PHPStdLib>>
function ldap_modify_batch(resource $link, string $dn, array $modifs);
<<__PHPStdLib>>
function ldap_bind(resource $link, $bind_rdn = null, $bind_password = null);
<<__PHPStdLib>>
function ldap_set_rebind_proc(resource $link, $callback);
<<__PHPStdLib>>
function ldap_sort(resource $link, resource $result, string $sortfilter);
<<__PHPStdLib>>
function ldap_start_tls(resource $link);
<<__PHPStdLib>>
function ldap_unbind(resource $link);
<<__PHPStdLib>>
function ldap_get_option(resource $link, int $option, inout $retval);
<<__PHPStdLib>>
function ldap_set_option($link, int $option, $newval);
<<__PHPStdLib>>
function ldap_close(resource $link);
<<__PHPStdLib>>
function ldap_list($link, $base_dn, $filter, $attributes = null, int $attrsonly = 0, int $sizelimit = -1, int $timelimit = -1, int $deref = -1);
<<__PHPStdLib>>
function ldap_read($link, $base_dn, $filter, $attributes = null, int $attrsonly = 0, int $sizelimit = -1, int $timelimit = -1, int $deref = -1);
<<__PHPStdLib>>
function ldap_search($link, $base_dn, $filter, $attributes = null, int $attrsonly = 0, int $sizelimit = -1, int $timelimit = -1, int $deref = -1);
<<__PHPStdLib>>
function ldap_rename(resource $link, string $dn, string $newrdn, string $newparent, bool $deleteoldrdn);
<<__PHPStdLib>>
function ldap_delete(resource $link, string $dn);
<<__PHPStdLib>>
function ldap_compare(resource $link, string $dn, string $attribute, string $value);
<<__PHPStdLib>>
function ldap_errno(resource $link);
<<__PHPStdLib>>
function ldap_error(resource $link);
<<__PHPStdLib>>
function ldap_get_dn(resource $link, resource $result_entry);
<<__PHPStdLib>>
function ldap_count_entries(resource $link, resource $result);
<<__PHPStdLib>>
function ldap_get_entries(resource $link, resource $result);
<<__PHPStdLib>>
function ldap_first_entry(resource $link, resource $result);
<<__PHPStdLib>>
function ldap_next_entry(resource $link, resource $result_entry);
<<__PHPStdLib>>
function ldap_get_attributes(resource $link, resource $result_entry);
<<__PHPStdLib>>
function ldap_first_attribute(resource $link, resource $result_entry);
<<__PHPStdLib>>
function ldap_next_attribute(resource $link, resource $result_entry);
<<__PHPStdLib>>
function ldap_first_reference(resource $link, resource $result);
<<__PHPStdLib>>
function ldap_next_reference(resource $link, resource $result_entry);
<<__PHPStdLib>>
function ldap_parse_reference(resource $link, resource $result_entry, inout $referrals);
<<__PHPStdLib>>
function ldap_parse_result(resource $link, resource $result, inout $errcode,
                           inout $matcheddn, inout $errmsg, inout $referrals);
<<__PHPStdLib>>
function ldap_free_result(resource $result);
<<__PHPStdLib>>
function ldap_get_values_len(resource $link, resource $result_entry, string $attribute);
<<__PHPStdLib>>
function ldap_get_values(resource $link, resource $result_entry, string $attribute);
<<__PHPStdLib>>
function ldap_control_paged_result(resource $link, int $pagesize, bool $iscritical = false, string $cookie = '');
<<__PHPStdLib>>
function ldap_control_paged_result_response(resource $link, resource $result,
                                            inout $cookie, inout $estimated);
<<__PHPStdLib>>
function ldap_escape(string $value, string $ignore = '', int $flags = 0);

const int LDAP_ESCAPE_FILTER = 1;
const int LDAP_ESCAPE_DN = 2;
const int LDAP_OPT_DEREF = 2;
const int LDAP_OPT_SIZELIMIT = 3;
const int LDAP_OPT_TIMELIMIT = 4;
const int LDAP_OPT_NETWORK_TIMEOUT = 20485;
const int LDAP_OPT_PROTOCOL_VERSION = 17;
const int LDAP_OPT_ERROR_NUMBER = 49;
const int LDAP_OPT_REFERRALS = 8;
const int LDAP_OPT_RESTART = 9;
const int LDAP_OPT_HOST_NAME = 48;
const int LDAP_OPT_ERROR_STRING = 50;
const int LDAP_OPT_MATCHED_DN = 51;
const int LDAP_OPT_SERVER_CONTROLS = 18;
const int LDAP_OPT_CLIENT_CONTROLS = 19;
