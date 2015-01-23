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
function ldap_connect($hostname = null, $port = 389) { }
function ldap_explode_dn($dn, $with_attrib) { }
function ldap_dn2ufn($db) { }
function ldap_err2str($errnum) { }
function ldap_add($link, $dn, $entry) { }
function ldap_mod_add($link, $dn, $entry) { }
function ldap_mod_del($link, $dn, $entry) { }
function ldap_mod_replace($link, $dn, $entry) { }
function ldap_modify($link, $dn, $entry) { }
function ldap_bind($link, $bind_rdn = null, $bind_password = null) { }
function ldap_set_rebind_proc($link, $callback) { }
function ldap_sort($link, $result, $sortfilter) { }
function ldap_start_tls($link) { }
function ldap_unbind($link) { }
function ldap_get_option($link, $option, &$retval) { }
function ldap_set_option($link, $option, $newval) { }
function ldap_close($link) { }
function ldap_list($link, $base_dn, $filter, $attributes = null, $attrsonly = 0, $sizelimit = -1, $timelimit = -1, $deref = -1) { }
function ldap_read($link, $base_dn, $filter, $attributes = null, $attrsonly = 0, $sizelimit = -1, $timelimit = -1, $deref = -1) { }
function ldap_search($link, $base_dn, $filter, $attributes = null, $attrsonly = 0, $sizelimit = -1, $timelimit = -1, $deref = -1) { }
function ldap_rename($link, $dn, $newrdn, $newparent, $deleteoldrdn) { }
function ldap_delete($link, $dn) { }
function ldap_compare($link, $dn, $attribute, $value) { }
function ldap_errno($link) { }
function ldap_error($link) { }
function ldap_get_dn($link, $result_entry) { }
function ldap_count_entries($link, $result) { }
function ldap_get_entries($link, $result) { }
function ldap_first_entry($link, $result) { }
function ldap_next_entry($link, $result_entry) { }
function ldap_get_attributes($link, $result_entry) { }
function ldap_first_attribute($link, $result_entry) { }
function ldap_next_attribute($link, $result_entry) { }
function ldap_first_reference($link, $result) { }
function ldap_next_reference($link, $result_entry) { }
function ldap_parse_reference($link, $result_entry, &$referrals) { }
function ldap_parse_result($link, $result, &$errcode, &$matcheddn = null, &$errmsg = null, &$referrals = null) { }
function ldap_free_result($result) { }
function ldap_get_values_len($link, $result_entry, $attribute) { }
function ldap_get_values($link, $result_entry, $attribute) { }
function ldap_control_paged_result($link, $pagesize, $iscritical = false, $cookie = '') { }
function ldap_control_paged_result_response($link, $result, &$cookie = null, &$estimated = null) { }
