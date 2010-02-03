<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('ldap_connect', Resource,
  array('hostname' => array(String, 'null_string'),
        'port' => array(Int32, '389')));

f('ldap_explode_dn', VariantVec,
  array('dn' => String,
        'with_attrib' => Int32));

f('ldap_dn2ufn', String,
  array('db' => String));

f('ldap_err2str', String,
  array('errnum' => Int32));

f('ldap_8859_to_t61', String,
  array('value' => String));

f('ldap_t61_to_8859', String,
  array('value' => String));

f('ldap_add', Boolean,
  array('link_identifier' => Resource,
        'dn' => String,
        'entry' => VariantVec));

f('ldap_mod_add', Boolean,
  array('link_identifier' => Resource,
        'dn' => String,
        'entry' => VariantVec));

f('ldap_mod_del', Boolean,
  array('link_identifier' => Resource,
        'dn' => String,
        'entry' => VariantVec));

f('ldap_mod_replace', Boolean,
  array('link_identifier' => Resource,
        'dn' => String,
        'entry' => VariantVec));

f('ldap_modify', Boolean,
  array('link_identifier' => Resource,
        'dn' => String,
        'entry' => VariantVec));

f('ldap_bind', Boolean,
  array('link_identifier' => Resource,
        'bind_rdn' => array(String, 'null_string'),
        'bind_password' => array(String, 'null_string')));

f('ldap_sasl_bind', Boolean,
  array('link' => Resource,
        'binddn' => array(String, 'null_string'),
        'password' => array(String, 'null_string'),
        'sasl_mech' => array(String, 'null_string'),
        'sasl_realm' => array(String, 'null_string'),
        'sasl_authz_id' => array(String, 'null_string'),
        'props' => array(String, 'null_string')));

f('ldap_set_rebind_proc', Boolean,
  array('link' => Resource,
        'callback' => String));

f('ldap_sort', Boolean,
  array('link' => Resource,
        'result' => Resource,
        'sortfilter' => String));

f('ldap_start_tls', Boolean,
  array('link' => Resource));

f('ldap_unbind', Boolean,
  array('link_identifier' => Resource));

f('ldap_get_option', Boolean,
  array('link_identifier' => Resource,
        'option' => Int32,
        'retval' => Variant | Reference));

f('ldap_set_option', Boolean,
  array('link_identifier' => Resource,
        'option' => Int32,
        'newval' => Variant));

f('ldap_close', Boolean,
  array('link_identifier' => Resource));

f('ldap_list', Resource,
  array('link_identifier' => Resource,
        'base_dn' => String,
        'filter' => String,
        'attributes' => array(StringVec, 'null'),
        'attrsonly' => array(Int32, '0'),
        'sizelimit' => array(Int32, '0'),
        'timelimit' => array(Int32, '0'),
        'deref' => array(Int32, '0')));

f('ldap_read', Resource,
  array('link_identifier' => Resource,
        'base_dn' => String,
        'filter' => String,
        'attributes' => array(StringVec, 'null'),
        'attrsonly' => array(Int32, '0'),
        'sizelimit' => array(Int32, '0'),
        'timelimit' => array(Int32, '0'),
        'deref' => array(Int32, '0')));

f('ldap_search', Resource,
  array('link_identifier' => Resource,
        'base_dn' => String,
        'filter' => String,
        'attributes' => array(StringVec, 'null'),
        'attrsonly' => array(Int32, '0'),
        'sizelimit' => array(Int32, '0'),
        'timelimit' => array(Int32, '0'),
        'deref' => array(Int32, '0')));

f('ldap_rename', Boolean,
  array('link_identifier' => Resource,
        'dn' => String,
        'newrdn' => String,
        'newparent' => String,
        'deleteoldrdn' => Boolean));

f('ldap_delete', Boolean,
  array('link_identifier' => Resource,
        'dn' => String));

f('ldap_compare', Variant,
  array('link_identifier' => Resource,
        'dn' => String,
        'attribute' => String,
        'value' => String));

f('ldap_errno', Int32,
  array('link_identifier' => Resource));

f('ldap_error', String,
  array('link_identifier' => Resource));

f('ldap_get_dn', String,
  array('link_identifier' => Resource,
        'result_entry_identifier' => Resource));

f('ldap_count_entries', Int32,
  array('link_identifier' => Resource,
        'result_identifier' => Resource));

f('ldap_get_entries', StringVec,
  array('link_identifier' => Resource,
        'result_identifier' => Resource));

f('ldap_first_entry', Resource,
  array('link_identifier' => Resource,
        'result_identifier' => Resource));

f('ldap_next_entry', Resource,
  array('link_identifier' => Resource,
        'result_identifier' => Resource));

f('ldap_get_attributes', StringVec,
  array('link_identifier' => Resource,
        'result_entry_identifier' => Resource));

f('ldap_first_attribute', String,
  array('link_identifier' => Resource,
        'result_identifier' => Resource));

f('ldap_next_attribute', String,
  array('link_identifier' => Resource,
        'result_entry_identifier' => Resource));

f('ldap_first_reference', Resource,
  array('link_identifier' => Resource,
        'result_identifier' => Resource));

f('ldap_next_reference', Resource,
  array('link_identifier' => Resource,
        'result_identifier' => Resource));

f('ldap_parse_reference', Boolean,
  array('link_identifier' => Resource,
        'result_identifier' => Resource,
        'referrals' => StringVec | Reference));

f('ldap_parse_result', Boolean,
  array('link_identifier' => Resource,
        'result_entry_identifier' => Resource,
        'errcode' => Int32 | Reference,
        'matcheddn' => array(String | Reference, 'null'),
        'errmsg' => array(String | Reference, 'null'),
        'referrals' => array(StringVec | Reference, 'null')));

f('ldap_free_result', Boolean,
  array('result_identifier' => Resource));

f('ldap_get_values_len', StringVec,
  array('link_identifier' => Resource,
        'result_entry_identifier' => Resource,
        'attribute' => String));

f('ldap_get_values', StringVec,
  array('link_identifier' => Resource,
        'result_entry_identifier' => Resource,
        'attribute' => String));
