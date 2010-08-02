<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// intl functions

f('intl_get_error_code', Int64, array());
f('intl_get_error_message', String, array());
f('intl_error_name', String, array('error_code' => Int64));
f('intl_is_failure', Boolean, array('error_code' => Int64));

///////////////////////////////////////////////////////////////////////////////
// Collator class

f('collator_asort', Variant,
  array('obj' => Variant,
        'arr' => VariantMap | Reference,
        'sort_flag' => array(Int64, '0')));
f('collator_compare', Variant,
  array('obj' => Variant,
        'str1' => String,
        'str2' => String));
f('collator_create', Variant,
  array('locale' => String));
f('collator_get_attribute', Variant,
  array('obj' => Variant,
        'attr' => Int64));
f('collator_get_error_code', Variant,
  array('obj' => Variant));
f('collator_get_error_message', Variant,
  array('obj' => Variant));
f('collator_get_locale', Variant,
  array('obj' => Variant,
        'type' => array(Int64, '0')));
f('collator_get_strength', Variant,
  array('obj' => Variant));
f('collator_set_attribute', Variant,
  array('obj' => Variant,
        'attr' => Int64,
        'val' => Int64));
f('collator_set_strength', Variant,
  array('obj' => Variant,
        'strength' => Int64));
f('collator_sort_with_sort_keys', Variant,
  array('obj' => Variant,
        'arr' => VariantMap | Reference));
f('collator_sort', Variant,
  array('obj' => Variant,
        'arr' => VariantMap | Reference,
        'sort_flag' => array(Int64, '0')));

c('Collator', null, array('Sweepable' => 'internal'),
  array(
    m(PublicMethod, '__construct', null,
      array('locale' => String)),
    m(PublicMethod, 'asort', Boolean,
      array('arr' => VariantMap | Reference,
            'sort_flag' => array(Int64, '0'))),
    m(PublicMethod, 'compare', Variant,
      array('str1' => String,
            'str2' => String)),
    m(PublicMethod | StaticMethod, 'create', Variant,
      array('locale' => String)),
    m(PublicMethod, 'getAttribute', Int64,
      array('attr' => Int64)),
    m(PublicMethod, 'getErrorCode', Int64,
      array()),
    m(PublicMethod, 'getErrorMessage', String,
      array()),
    m(PublicMethod, 'getLocale', String,
      array('type' => array(Int64, '0'))),
    m(PublicMethod, 'getStrength', Int64,
      array()),
    m(PublicMethod, 'setAttribute', Boolean,
      array('attr' => Int64,
            'val' => Int64)),
    m(PublicMethod, 'setStrength', Boolean,
      array('strength' => Int64)),
    m(PublicMethod, 'sortWithSortKeys', Boolean,
      array('arr' => VariantMap | Reference)),
    m(PublicMethod, 'sort', Boolean,
      array('arr' => VariantMap | Reference,
            'sort_flag' => array(Int64, '0'))),
    ),
  array(
    ck("SORT_REGULAR", Int64),
    ck("SORT_NUMERIC", Int64),
    ck("SORT_STRING", Int64),
    ck("FRENCH_COLLATION", Int64),
    ck("ALTERNATE_HANDLING", Int64),
    ck("CASE_FIRST", Int64),
    ck("CASE_LEVEL", Int64),
    ck("NORMALIZATION_MODE", Int64),
    ck("STRENGTH", Int64),
    ck("HIRAGANA_QUATERNARY_MODE", Int64),
    ck("NUMERIC_COLLATION", Int64),
    ck("DEFAULT_VALUE", Int64),
    ck("PRIMARY", Int64),
    ck("SECONDARY", Int64),
    ck("TERTIARY", Int64),
    ck("DEFAULT_STRENGTH", Int64),
    ck("QUATERNARY", Int64),
    ck("IDENTICAL", Int64),
    ck("OFF", Int64),
    ck("ON", Int64),
    ck("SHIFTED", Int64),
    ck("NON_IGNORABLE", Int64),
    ck("LOWER_FIRST", Int64),
    ck("UPPER_FIRST", Int64),
    ),
  "\n".
  " private:\n".
  "  String     m_locale;\n".
  "  UCollator *m_ucoll;\n".
  "  intl_error m_errcode;"
  );

///////////////////////////////////////////////////////////////////////////////
// Locale class

c('Locale', null, array(),
  // Methods
  array(m(PublicMethod, '__construct', null, array())),
  // Constants
  array(
    ck("ACTUAL_LOCALE", Int64),
    ck("VALID_LOCALE",  Int64),
    )
  );

///////////////////////////////////////////////////////////////////////////////
// Normalizer class

c('Normalizer', null, array('Sweepable' => 'internal'),
  array(m(PublicMethod, '__construct', null, array()),
        m(PublicMethod | StaticMethod, 'isNormalized', Variant,
          array('input' => String,
                'form' => array(Int64, 'q_normalizer_FORM_C'))),
        m(PublicMethod | StaticMethod, 'normalize', Variant,
          array('input' => String,
                'form' => array(Int64, 'q_normalizer_FORM_C'))),
       ),
  array(
    ck("NONE",    Int64),
    ck("FORM_D",  Int64),
    ck("NFD",     Int64),
    ck("FORM_KD", Int64),
    ck("NFKD",    Int64),
    ck("FORM_C",  Int64),
    ck("NFC",     Int64),
    ck("FORM_KC", Int64),
    ck("NFKC",    Int64),
  )
 );

///////////////////////////////////////////////////////////////////////////////
// idn functions

f('idn_to_ascii', Variant,
  array('domain' => String,
        'errorcode' => array(Variant | Reference, 'null')));

f('idn_to_unicode', Variant,
  array('domain' => String,
        'errorcode' => array(Variant | Reference, 'null')));

f('idn_to_utf8', Variant,
  array('domain' => String,
        'errorcode' => array(Variant | Reference, 'null')));
