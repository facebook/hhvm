<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function ldap_connect(
  HH\FIXME\MISSING_PARAM_TYPE $hostname = null,
  int $port = 389,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_explode_dn(
  string $dn,
  int $with_attrib,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_dn2ufn(string $db): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_err2str(int $errnum): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_add(
  resource $link,
  string $dn,
  HH\FIXME\MISSING_PARAM_TYPE $entry,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_mod_add(
  resource $link,
  string $dn,
  HH\FIXME\MISSING_PARAM_TYPE $entry,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_mod_del(
  resource $link,
  string $dn,
  HH\FIXME\MISSING_PARAM_TYPE $entry,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_mod_replace(
  resource $link,
  string $dn,
  HH\FIXME\MISSING_PARAM_TYPE $entry,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_modify(
  resource $link,
  string $dn,
  HH\FIXME\MISSING_PARAM_TYPE $entry,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_modify_batch(
  resource $link,
  string $dn,
  varray<darray<string, mixed>> $modifs,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_bind(
  resource $link,
  HH\FIXME\MISSING_PARAM_TYPE $bind_rdn = null,
  HH\FIXME\MISSING_PARAM_TYPE $bind_password = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_set_rebind_proc(
  resource $link,
  HH\FIXME\MISSING_PARAM_TYPE $callback,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_sort(
  resource $link,
  resource $result,
  string $sortfilter,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_start_tls(resource $link): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_unbind(resource $link): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_get_option(
  resource $link,
  int $option,
  inout $retval,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_set_option(
  HH\FIXME\MISSING_PARAM_TYPE $link,
  int $option,
  HH\FIXME\MISSING_PARAM_TYPE $newval,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_close(resource $link): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_list(
  HH\FIXME\MISSING_PARAM_TYPE $link,
  HH\FIXME\MISSING_PARAM_TYPE $base_dn,
  HH\FIXME\MISSING_PARAM_TYPE $filter,
  HH\FIXME\MISSING_PARAM_TYPE $attributes = null,
  int $attrsonly = 0,
  int $sizelimit = -1,
  int $timelimit = -1,
  int $deref = -1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_read(
  HH\FIXME\MISSING_PARAM_TYPE $link,
  HH\FIXME\MISSING_PARAM_TYPE $base_dn,
  HH\FIXME\MISSING_PARAM_TYPE $filter,
  HH\FIXME\MISSING_PARAM_TYPE $attributes = null,
  int $attrsonly = 0,
  int $sizelimit = -1,
  int $timelimit = -1,
  int $deref = -1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_search(
  HH\FIXME\MISSING_PARAM_TYPE $link,
  HH\FIXME\MISSING_PARAM_TYPE $base_dn,
  HH\FIXME\MISSING_PARAM_TYPE $filter,
  HH\FIXME\MISSING_PARAM_TYPE $attributes = null,
  int $attrsonly = 0,
  int $sizelimit = -1,
  int $timelimit = -1,
  int $deref = -1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_rename(
  resource $link,
  string $dn,
  string $newrdn,
  string $newparent,
  bool $deleteoldrdn,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_delete(resource $link, string $dn): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_compare(
  resource $link,
  string $dn,
  string $attribute,
  string $value,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_errno(resource $link): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_error(resource $link): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_get_dn(
  resource $link,
  resource $result_entry,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_count_entries(
  resource $link,
  resource $result,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_get_entries(
  resource $link,
  resource $result,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_first_entry(
  resource $link,
  resource $result,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_next_entry(
  resource $link,
  resource $result_entry,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_get_attributes(
  resource $link,
  resource $result_entry,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_first_attribute(
  resource $link,
  resource $result_entry,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_next_attribute(
  resource $link,
  resource $result_entry,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_first_reference(
  resource $link,
  resource $result,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_next_reference(
  resource $link,
  resource $result_entry,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_parse_reference(
  resource $link,
  resource $result_entry,
  inout $referrals,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_parse_result(
  resource $link,
  resource $result,
  inout $errcode,
  inout $matcheddn,
  inout $errmsg,
  inout $referrals,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_free_result(resource $result): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_get_values_len(
  resource $link,
  resource $result_entry,
  string $attribute,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_get_values(
  resource $link,
  resource $result_entry,
  string $attribute,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_control_paged_result(
  resource $link,
  int $pagesize,
  bool $iscritical = false,
  string $cookie = '',
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_control_paged_result_response(
  resource $link,
  resource $result,
  inout $cookie,
  inout $estimated,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ldap_escape(
  string $value,
  string $ignore = '',
  int $flags = 0,
): HH\FIXME\MISSING_RETURN_TYPE;

const int LDAP_ESCAPE_FILTER;
const int LDAP_ESCAPE_DN;
const int LDAP_OPT_DEREF;
const int LDAP_OPT_SIZELIMIT;
const int LDAP_OPT_TIMELIMIT;
const int LDAP_OPT_NETWORK_TIMEOUT;
const int LDAP_OPT_PROTOCOL_VERSION;
const int LDAP_OPT_ERROR_NUMBER;
const int LDAP_OPT_REFERRALS;
const int LDAP_OPT_RESTART;
const int LDAP_OPT_HOST_NAME;
const int LDAP_OPT_ERROR_STRING;
const int LDAP_OPT_MATCHED_DN;
const int LDAP_OPT_SERVER_CONTROLS;
const int LDAP_OPT_CLIENT_CONTROLS;
