<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('ldap_connect', Variant,
  array('hostname' => array(String, 'null_string'),
        'port' => array(Int32, '389')));

f('ldap_explode_dn', Variant,
  array('dn' => String,
        'with_attrib' => Int32));

f('ldap_dn2ufn', Variant,
  array('db' => String));

f('ldap_err2str', String,
  array('errnum' => Int32));

f('ldap_add', Boolean,
  array('link' => Resource,
        'dn' => String,
        'entry' => VariantVec));

f('ldap_mod_add', Boolean,
  array('link' => Resource,
        'dn' => String,
        'entry' => VariantVec));

f('ldap_mod_del', Boolean,
  array('link' => Resource,
        'dn' => String,
        'entry' => VariantVec));

f('ldap_mod_replace', Boolean,
  array('link' => Resource,
        'dn' => String,
        'entry' => VariantVec));

f('ldap_modify', Boolean,
  array('link' => Resource,
        'dn' => String,
        'entry' => VariantVec));

f('ldap_bind', Boolean,
  array('link' => Resource,
        'bind_rdn' => array(String, 'null_string'),
        'bind_password' => array(String, 'null_string')));

f('ldap_set_rebind_proc', Boolean,
  array('link' => Resource,
        'callback' => Variant));

f('ldap_sort', Boolean,
  array('link' => Resource,
        'result' => Resource,
        'sortfilter' => String));

f('ldap_start_tls', Boolean,
  array('link' => Resource));

f('ldap_unbind', Boolean,
  array('link' => Resource));

f('ldap_get_option', Boolean,
  array('link' => Resource,
        'option' => Int32,
        'retval' => Variant | Reference));

f('ldap_set_option', Boolean,
  array('link' => Variant,
        'option' => Int32,
        'newval' => Variant));

f('ldap_close', Boolean,
  array('link' => Resource));

f('ldap_list', Variant,
  array('link' => Variant,
        'base_dn' => Variant,
        'filter' => Variant,
        'attributes' => array(StringVec, 'null_array'),
        'attrsonly' => array(Int32, '0'),
        'sizelimit' => array(Int32, '-1'),
        'timelimit' => array(Int32, '-1'),
        'deref' => array(Int32, '-1')));

f('ldap_read', Variant,
  array('link' => Variant,
        'base_dn' => Variant,
        'filter' => Variant,
        'attributes' => array(StringVec, 'null_array'),
        'attrsonly' => array(Int32, '0'),
        'sizelimit' => array(Int32, '-1'),
        'timelimit' => array(Int32, '-1'),
        'deref' => array(Int32, '-1')));

f('ldap_search', Variant,
  array('link' => Variant,
        'base_dn' => Variant,
        'filter' => Variant,
        'attributes' => array(StringVec, 'null_array'),
        'attrsonly' => array(Int32, '0'),
        'sizelimit' => array(Int32, '-1'),
        'timelimit' => array(Int32, '-1'),
        'deref' => array(Int32, '-1')));

f('ldap_rename', Boolean,
  array('link' => Resource,
        'dn' => String,
        'newrdn' => String,
        'newparent' => String,
        'deleteoldrdn' => Boolean));

f('ldap_delete', Boolean,
  array('link' => Resource,
        'dn' => String));

f('ldap_compare', Variant,
  array('link' => Resource,
        'dn' => String,
        'attribute' => String,
        'value' => String));

f('ldap_errno', Int32,
  array('link' => Resource));

f('ldap_error', String,
  array('link' => Resource));

f('ldap_get_dn', Variant,
  array('link' => Resource,
        'result_entry' => Resource));

f('ldap_count_entries', Int32,
  array('link' => Resource,
        'result' => Resource));

f('ldap_get_entries', Variant,
  array('link' => Resource,
        'result' => Resource));

f('ldap_first_entry', Variant,
  array('link' => Resource,
        'result' => Resource));

f('ldap_next_entry', Variant,
  array('link' => Resource,
        'result_entry' => Resource));

f('ldap_get_attributes', StringVec,
  array('link' => Resource,
        'result_entry' => Resource));

f('ldap_first_attribute', Variant,
  array('link' => Resource,
        'result_entry' => Resource));

f('ldap_next_attribute', Variant,
  array('link' => Resource,
        'result_entry' => Resource));

f('ldap_first_reference', Variant,
  array('link' => Resource,
        'result' => Resource));

f('ldap_next_reference', Variant,
  array('link' => Resource,
        'result_entry' => Resource));

f('ldap_parse_reference', Boolean,
  array('link' => Resource,
        'result_entry' => Resource,
        'referrals' => StringVec | Reference));

f('ldap_parse_result', Boolean,
  array('link' => Resource,
        'result' => Resource,
        'errcode' => Int32 | Reference,
        'matcheddn' => array(String | Reference, 'null'),
        'errmsg' => array(String | Reference, 'null'),
        'referrals' => array(StringVec | Reference, 'null')));

f('ldap_free_result', Boolean,
  array('result' => Resource));

f('ldap_get_values_len', Variant,
  array('link' => Resource,
        'result_entry' => Resource,
        'attribute' => String));

f('ldap_get_values', Variant,
  array('link' => Resource,
        'result_entry' => Resource,
        'attribute' => String));
