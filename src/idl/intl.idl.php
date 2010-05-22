<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// Normalizer

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
