<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

<<__PHPStdLib>>
function ldap_connect($hostname = null, $port = 389) { }
<<__PHPStdLib>>
function ldap_explode_dn($dn, $with_attrib) { }
<<__PHPStdLib>>
function ldap_dn2ufn($db) { }
<<__PHPStdLib>>
function ldap_err2str($errnum) { }
<<__PHPStdLib>>
function ldap_add($link, $dn, $entry) { }
<<__PHPStdLib>>
function ldap_mod_add($link, $dn, $entry) { }
<<__PHPStdLib>>
function ldap_mod_del($link, $dn, $entry) { }
<<__PHPStdLib>>
function ldap_mod_replace($link, $dn, $entry) { }
<<__PHPStdLib>>
function ldap_modify($link, $dn, $entry) { }
<<__PHPStdLib>>
function ldap_modify_batch($link, string $dn, array $modifs): bool { return false; }
<<__PHPStdLib>>
function ldap_bind($link, $bind_rdn = null, $bind_password = null) { }
<<__PHPStdLib>>
function ldap_set_rebind_proc($link, $callback) { }
<<__PHPStdLib>>
function ldap_sort($link, $result, $sortfilter) { }
<<__PHPStdLib>>
function ldap_start_tls($link) { }
<<__PHPStdLib>>
function ldap_unbind($link) { }
<<__PHPStdLib>>
function ldap_get_option($link, $option, &$retval) { }
<<__PHPStdLib>>
function ldap_set_option($link, $option, $newval) { }
<<__PHPStdLib>>
function ldap_close($link) { }
<<__PHPStdLib>>
function ldap_list($link, $base_dn, $filter, $attributes = null, $attrsonly = 0, $sizelimit = -1, $timelimit = -1, $deref = -1) { }
<<__PHPStdLib>>
function ldap_read($link, $base_dn, $filter, $attributes = null, $attrsonly = 0, $sizelimit = -1, $timelimit = -1, $deref = -1) { }
<<__PHPStdLib>>
function ldap_search($link, $base_dn, $filter, $attributes = null, $attrsonly = 0, $sizelimit = -1, $timelimit = -1, $deref = -1) { }
<<__PHPStdLib>>
function ldap_rename($link, $dn, $newrdn, $newparent, $deleteoldrdn) { }
<<__PHPStdLib>>
function ldap_delete($link, $dn) { }
<<__PHPStdLib>>
function ldap_compare($link, $dn, $attribute, $value) { }
<<__PHPStdLib>>
function ldap_errno($link) { }
<<__PHPStdLib>>
function ldap_error($link) { }
<<__PHPStdLib>>
function ldap_get_dn($link, $result_entry) { }
<<__PHPStdLib>>
function ldap_count_entries($link, $result) { }
<<__PHPStdLib>>
function ldap_get_entries($link, $result) { }
<<__PHPStdLib>>
function ldap_first_entry($link, $result) { }
<<__PHPStdLib>>
function ldap_next_entry($link, $result_entry) { }
<<__PHPStdLib>>
function ldap_get_attributes($link, $result_entry) { }
<<__PHPStdLib>>
function ldap_first_attribute($link, $result_entry) { }
<<__PHPStdLib>>
function ldap_next_attribute($link, $result_entry) { }
<<__PHPStdLib>>
function ldap_first_reference($link, $result) { }
<<__PHPStdLib>>
function ldap_next_reference($link, $result_entry) { }
<<__PHPStdLib>>
function ldap_parse_reference($link, $result_entry, &$referrals) { }
<<__PHPStdLib>>
function ldap_parse_result($link, $result, &$errcode, &$matcheddn = null, &$errmsg = null, &$referrals = null) { }
<<__PHPStdLib>>
function ldap_free_result($result) { }
<<__PHPStdLib>>
function ldap_get_values_len($link, $result_entry, $attribute) { }
<<__PHPStdLib>>
function ldap_get_values($link, $result_entry, $attribute) { }
<<__PHPStdLib>>
function ldap_control_paged_result($link, $pagesize, $iscritical = false, $cookie = '') { }
<<__PHPStdLib>>
function ldap_control_paged_result_response($link, $result, &$cookie = null, &$estimated = null) { }
<<__PHPStdLib>>
function ldap_escape(string $value, string $ignore = '', int $flags = 0) { }

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
