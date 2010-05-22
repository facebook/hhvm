<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('checkdate', Boolean,
  array('month' => Int32,
        'day' => Int32,
        'year' => Int32));

f('date_create', Object,
  array('time' => array(String, 'null_string'),
        'timezone' => array(Object, 'null_object')));

f('date_date_set', NULL,
  array('object' => Object,
        'year' => Int32,
        'month' => Int32,
        'day' => Int32));

f('date_default_timezone_get', String);

f('date_default_timezone_set', Boolean,
  array('name' => String));

f('date_format', String,
  array('object' => Object,
        'format' => String));

f('date_isodate_set', NULL,
  array('object' => Object,
        'year' => Int32,
        'week' => Int32,
        'day' => array(Int32, '1')));

f('date_modify', NULL,
  array('object' => Object,
        'modify' => String));

f('date_offset_get', Int32,
  array('object' => Object));

f('date_parse', Variant,
  array('date' => String));

f('date_sun_info', VariantMap,
  array('ts' => Int64,
        'latitude' => Double,
        'longitude' => Double));

f('date_sunrise', Variant,
  array('timestamp' => Int64,
        'format' => array(Int32, '0'),
        'latitude' => array(Double, '0.0'),
        'longitude' => array(Double, '0.0'),
        'zenith' => array(Double, '0.0'),
        'gmt_offset' => array(Double, '99999.0')));

f('date_sunset', Variant,
  array('timestamp' => Int64,
        'format' => array(Int32, '0'),
        'latitude' => array(Double, '0.0'),
        'longitude' => array(Double, '0.0'),
        'zenith' => array(Double, '0.0'),
        'gmt_offset' => array(Double, '99999.0')));

f('date_time_set', NULL,
  array('object' => Object,
        'hour' => Int32,
        'minute' => Int32,
        'second' => array(Int32, '0')));

f('date_timezone_get', Variant,
  array('object' => Object));

f('date_timezone_set', NULL,
  array('object' => Object,
        'timezone' => Object));

f('date', Variant,
  array('format' => String,
        'timestamp' => array(Int64, 'TimeStamp::Current()')));

f('getdate', VariantMap,
  array('timestamp' => array(Int64, 'TimeStamp::Current()')));

f('gettimeofday', Variant,
  array('return_float' => array(Boolean, 'false')));

f('gmdate', Variant,
  array('format' => String,
        'timestamp' => array(Int64, 'TimeStamp::Current()')));

f('gmmktime', Variant,
  array('hour' => array(Int32, 'INT_MAX'),
        'minute' => array(Int32, 'INT_MAX'),
        'second' => array(Int32, 'INT_MAX'),
        'month' => array(Int32, 'INT_MAX'),
        'day' => array(Int32, 'INT_MAX'),
        'year' => array(Int32, 'INT_MAX')));

f('gmstrftime', String,
  array('format' => String,
        'timestamp' => array(Int64, 'TimeStamp::Current()')));

f('idate', Variant,
  array('format' => String,
        'timestamp' => array(Int64, 'TimeStamp::Current()')));

f('localtime', VariantMap,
  array('timestamp' => array(Int64, 'TimeStamp::Current()'),
        'is_associative' => array(Boolean, 'false')));

f('microtime', Variant,
  array('get_as_float' => array(Boolean, 'false')));

f('mktime', Variant,
  array('hour' => array(Int32, 'INT_MAX'),
        'minute' => array(Int32, 'INT_MAX'),
        'second' => array(Int32, 'INT_MAX'),
        'month' => array(Int32, 'INT_MAX'),
        'day' => array(Int32, 'INT_MAX'),
        'year' => array(Int32, 'INT_MAX')));

f('strftime', Variant,
  array('format' => String,
        'timestamp' => array(Int64, 'TimeStamp::Current()')));

f('strptime', Variant,
  array('date' => String,
        'format' => String));

f('strtotime', Variant,
  array('input' => String,
        'timestamp' => array(Int64, 'TimeStamp::Current()')));

f('time', Int32);

f('timezone_abbreviations_list', StringVec);

f('timezone_identifiers_list', StringVec);

f('timezone_name_from_abbr', Variant,
  array('abbr' => String,
        'gmtOffset' => array(Int32, '-1'),
        'isdst' => array(Boolean, 'true')));

f('timezone_name_get', String,
  array('object' => Object));

f('timezone_offset_get', Int32,
  array('object' => Object,
        'dt' => Object));

f('timezone_open', Object,
  array('timezone' => String));

f('timezone_transitions_get', VariantVec,
  array('object' => Object));

c('DateTime', null, array(),
  array(
    m(PublicMethod, '__construct', null,
      array('time' => array(String, '"now"'),
            'timezone' => array(Object, 'null_object'))),
    m(PublicMethod, 'format', String,
      array('format' => String)),
    m(PublicMethod, 'getOffset', Int64),
    m(PublicMethod, 'getTimezone', Variant),
    m(PublicMethod, 'modify', Object,
      array('modify' => String)),
    m(PublicMethod, 'setDate', Object,
      array('year' => Int64,
            'month' => Int64,
            'day' => Int64)),
    m(PublicMethod, 'setISODate', Object,
      array('year' => Int64,
            'week' => Int64,
            'day' => array(Int64, '1'))),
    m(PublicMethod, 'setTime', Object,
      array('hour' => Int64,
            'minute' => Int64,
            'second' => array(Int64, '0'))),
    m(PublicMethod, 'setTimezone', Object,
      array('timezone' => Object))
    ),
  array(
    ck("ATOM", String),
    ck("COOKIE", String),
    ck("ISO8601", String),
    ck("RFC822", String),
    ck("RFC850", String),
    ck("RFC1036", String),
    ck("RFC1123", String),
    ck("RFC2822", String),
    ck("RFC3339", String),
    ck("RSS", String),
    ck("W3C", String)
    ),
  "\n".
  " private:\n".
  "  SmartObject<DateTime> m_dt;"
  );

c('DateTimeZone', null, array(),
  array(
    m(PublicMethod, '__construct', null,
      array('timezone' => String)),
    m(PublicMethod, 'getName', String),
    m(PublicMethod, 'getOffset', Int64,
      array('datetime' => Object)),
    m(PublicMethod, 'getTransitions', VariantMap),
    m(PublicMethod | StaticMethod, 'listAbbreviations', VariantMap),
    m(PublicMethod | StaticMethod, 'listIdentifiers', VariantMap)
    ),
  array(
    ck("AFRICA", Int64),
    ck("AMERICA", Int64),
    ck("ANTARCTICA", Int64),
    ck("ARCTIC", Int64),
    ck("ASIA", Int64),
    ck("ATLANTIC", Int64),
    ck("AUSTRALIA", Int64),
    ck("EUROPE", Int64),
    ck("INDIAN", Int64),
    ck("PACIFIC", Int64),
    ck("UTC", Int64),
    ck("ALL", Int64),
    ck("ALL_WITH_BC", Int64),
    ck("PER_COUNTRY", Int64)
    ),
  "\n".
  " private:\n".
  "  SmartObject<TimeZone> m_tz;"
  );

