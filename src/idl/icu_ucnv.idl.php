<?php
/**
 * @nolint
 */
DefinePreamble(<<<CPP

#include <runtime/base/zend/zend_collator.h>

// Avoid dragging in the icu namespace.
#ifndef U_USING_ICU_NAMESPACE
#define U_USING_ICU_NAMESPACE 0
#endif

#include <unicode/utypes.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>
CPP
);

BeginClass(
  array(
    'name'   => "UConverter",
    'desc'   => "ICU UConverter class",
    'flags'  => HasDocComment,
    'footer' => <<<CPP
  private:
    static void throwFailure(UErrorCode error,
                             const char *fname,
                             intl_error &merror);
    bool checkLimits(int64_t available, int64_t needed);
    void appendToUTarget(Variant val, UConverterToUnicodeArgs *args);
    void appendFromUTarget(Variant val, UConverterFromUnicodeArgs *args);
    static void ucnvToUCallback(c_UConverter *objval,
                                UConverterToUnicodeArgs *args,
                                const char *codeUnits, int32_t length,
                                UConverterCallbackReason reason,
                                UErrorCode *pErrorCode);
    static void ucnvFromUCallback(c_UConverter *objval,
                                  UConverterFromUnicodeArgs *args,
                                  const UChar *codeUnits, int32_t length,
                                  UChar32 codePoint,
                                  UConverterCallbackReason reason,
                                  UErrorCode *pErrorCode);
    static bool setEncoding(CStrRef encoding,
                            UConverter **pcnv,
                            intl_error &err);
    static bool setSubstChars(String chars, UConverter *cnv, intl_error &err);
    bool setCallback(UConverter *cnv);
    Variant defaultCallback(int64 reason, VRefParam error);
    static String doConvert(CStrRef str, UConverter *toCnv,
                            UConverter *fromCnv, intl_error &err);
  public:
    intl_error  m_error;
    UConverter *m_src;
    UConverter *m_dest;
CPP
  )
);

DefineConstant(array(
  'name'   => "REASON_UNASSIGNED",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "REASON_ILLEGAL",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "REASON_IRREGULAR",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "REASON_RESET",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "REASON_CLOSE",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "REASON_CLONE",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "UNSUPPORTED_CONVERTER",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "SBCS",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "DBCS",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "MBCS",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "LATIN_1",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "UTF8",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "UTF16_BigEndian",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "UTF16_LittleEndian",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "UTF32_BigEndian",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "UTF32_LittleEndian",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "EBCDIC_STATEFUL",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "ISO_2022",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "LMBCS_1",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "LMBCS_2",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "LMBCS_3",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "LMBCS_4",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "LMBCS_5",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "LMBCS_6",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "LMBCS_8",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "LMBCS_11",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "LMBCS_16",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "LMBCS_17",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "LMBCS_18",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "LMBCS_19",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "LMBCS_LAST",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "HZ",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "SCSU",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "ISCII",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "US_ASCII",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "UTF7",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "BOCU1",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "UTF16",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "UTF32",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "CESU8",
  'type'   => Int64,
));

DefineConstant(array(
  'name'   => "IMAP_MAILBOX",
  'type'   => Int64,
));

DefineFunction(
  array(
    'name'   => "__construct",
    'desc'   => "Object constructor",
    'flags'  => HasDocComment,
    'args'   => array(
      array(
        'name'   => "toEncoding",
        'type'   => String,
        'value'  => "\"utf-8\"",
        'desc'   => "Target character encoding",
      ),
      array(
        'name'   => "fromEncoding",
        'type'   => String,
        'value'  => "\"utf-8\"",
        'desc'   => "Source character encoding",
      ),
    ),
  )
);

DefineFunction(
  array(
    'name'   => "__destruct",
    'desc'   => "Object destructor",
    'return' => array(
      'type'   => Variant,
    ),
  )
);

/* Get/Set Source/Destination encodings */

DefineFunction(
  array(
    'name'   => "getSourceEncoding",
    'desc'   => "Returns the name of the source encoding",
    'flags'  => HasDocComment,
    'return' => array(
      'type'   => String,
      'desc'   => "Cannonical name of source encoding",
    ),
  )
);

DefineFunction(
  array(
    'name'   => "setSourceEncoding",
    'desc'   => "Changes the source encoding converter to the named encoding",
    'flags'  => HasDocComment,
    'args'   => array(
      array(
        'name'   => 'encoding',
        'type'   => String,
        'desc'   => "Name of encoding to use",
      ),
    ),
  )
);

DefineFunction(
  array(
    'name'   => "getDestinationEncoding",
    'desc'   => "Returns the name of the destination encoding",
    'flags'  => HasDocComment,
    'return' => array(
      'type'   => String,
      'desc'   => "Cannonical name of destination encoding",
    ),
  )
);

DefineFunction(
  array(
    'name'   => "setDestinationEncoding",
    'desc'   => "Changes the destination encoding converter to the named encoding",
    'flags'  => HasDocComment,
    'args'   => array(
      array(
        'name'   => 'encoding',
        'type'   => String,
        'desc'   => "Name of encoding to use",
      ),
    ),
  )
);

/* Get algorithmic types */

DefineFunction(
  array(
    'name'   => "getSourceType",
    'desc'   => "Returns the source algorithmic encoding type (e.g. SBCS, DBCS, LATIN_1, UTF8, etc...)",
    'flags'  => HasDocComment,
    'return' => array(
      'type'   => Int64,
      'desc'   => "Algorithmic encoding type",
    ),
  )
);

DefineFunction(
  array(
    'name'   => "getDestinationType",
    'desc'   => "Returns the destination algorithmic encoding type (e.g. SBCS, DBCS, LATIN_1, UTF8, etc...)",
    'flags'  => HasDocComment,
    'return' => array(
      'type'   => Int64,
      'desc'   => "Algorithmic encoding type",
    ),
  )
);

/* Basic character substitution */

DefineFunction(
  array(
    'name'   => "getSubstChars",
    'desc'   => "Returns the current substitution character used for conversion failures",
    'flags'  => HasDocComment,
    'return' => array(
      'type'   => String,
      'desc'   => "One or more codeunits representing a single codepoint",
    ),
  )
);

DefineFunction(
  array(
    'name'   => "setSubstChars",
    'desc'   => "Set the substitution character to use for conversion failures",
    'flags'  => HasDocComment,
    'return' => array(
      'type'   => Boolean,
      'desc'   => "Whether or not setting substitution characters succeeded",
    ),
    'args'   => array(
      array(
        'name'   => "chars",
        'type'   => String,
        'desc'   => "One or more codeunits representing a codepoint",
      ),
    ),
  )
);

/* Standard callbacks */

DefineFunction(
  array(
    'name'   => "fromUCallback",
    'desc'   => "Issued by the object when converting to the target encoding",
    'flags'  => HasDocComment,
    'return' => array(
      'type'   => Variant,
      'desc'   => "Substitution codeunits for illegal/irregular/unassigned codepoints",
    ),
    'args'   => array(
      array(
        'name'   => "reason",
        'type'   => Int64,
        'desc'   => "Event which caused the callback",
      ),
      array(
        'name'   => "source",
        'type'   => Int64Vec,
        'desc'   => "Contextual codepoints from the string being converted",
      ),
      array(
        'name'   => "codepoint",
        'type'   => Int64,
        'desc'   => "The specific codepoint in question",
      ),
      array(
        'name'   => "error",
        'type'   => Variant,
        'ref'    => true,
        'desc'   => "Reference param, error condition on the way in, should be U_ZERO_ERROR on the way out",
      ),
    ),
  )
);

DefineFunction(
  array(
    'name'   => "toUCallback",
    'desc'   => "Issued by the object when converting from the source encoding",
    'flags'  => HasDocComment,
    'return' => array(
      'type'   => Variant,
      'desc'   => "Substitution codepoints for illegal/irregular/unassigned codeunits",
    ),
    'args'   => array(
      array(
        'name'   => "reason",
        'type'   => Int64,
        'desc'   => "Event which caused the callback",
      ),
      array(
        'name'   => "source",
        'type'   => String,
        'desc'   => "Contextual codeunits from the string being converted",
      ),
      array(
        'name'   => "codeunits",
        'type'   => String,
        'desc'   => "The specific codeunits in question",
      ),
      array(
        'name'   => "error",
        'type'   => Variant,
        'ref'    => true,
        'desc'   => "Reference param, error condition on the way in, should be U_ZERO_ERROR on the way out",
      ),
    ),
  )
);

/* Primary converter functions */

DefineFunction(
  array(
    'name'   => "convert",
    'desc'   => "Convert a string between the source/destination encodings",
    'flags'  => HasDocComment,
    'return' => array(
      'type'   => Variant,
      'desc'   => "Transcoded string",
    ),
    'args'   => array(
      array(
        'name'   => "str",
        'type'   => String,
        'desc'   => "String to be transcoded",
      ),
      array(
        'name'   => "reverse",
        'type'   => Boolean,
        'value'  => "false",
        'desc'   => "Convert from destination to source encodings instead",
      ),
    ),
  )
);

DefineFunction(
  array(
    'name'   => "transcode",
    'desc'   => "Convert a string between two encodings",
    'flags'  => HasDocComment|IsStatic,
    'return' => array(
      'type'   => Variant,
      'desc'   => "Transcoded string",
    ),
    'args'   => array(
      array(
        'name'   => "str",
        'type'   => String,
        'desc'   => "String to be transcoded",
      ),
      array(
        'name'   => "toEncoding",
        'type'   => String,
        'desc'   => "Source encoding",
      ),
      array(
        'name'   => "fromEncoding",
        'type'   => String,
        'desc'   => "Destination encoding",
      ),
      array(
        'name'   => "options",
        'type'   => StringMap,
        'value'  => "null_variant",
        'desc'   => "Optional configuration for converters",
      ),
    ),
  )
);

/* ext/intl style error handlers */

DefineFunction(
  array(
    'name'   => "getErrorCode",
    'desc'   => "Last UErrorCode associated with this converter",
    'flags'  => HasDocComment,
    'return' => array(
      'type'   => Int64,
      'desc'   => "UErrorCode U_* value",
    ),
  )
);

DefineFunction(
  array(
    'name'   => "getErrorMessage",
    'desc'   => "Last error message associated with this converter",
    'flags'  => HasDocComment,
    'return' => array(
      'type'   => String,
      'desc'   => "Descriptive error message",
    ),
  )
);

/* Ennumerators and lookups */

DefineFunction(
  array(
    'name'   => "reasonText",
    'desc'   => "Name of REASON_* constant",
    'flags'  => HasDocComment|IsStatic,
    'return' => array(
      'type'   => String,
      'desc'   => "Printable REASON_* constant name",
    ),
    'args'   => array(
      array(
        'name'   => "reason",
        'type'   => Int64,
        'desc'   => "REASON_* constant",
      ),
    ),
  )
);

DefineFunction(
  array(
    'name'   => "getAvailable",
    'desc'   => "Returns list of available encodings",
    'flags'  => HasDocComment|IsStatic,
    'return' => array(
      'type'   => StringVec,
      'desc'   => "Available encodings",
    ),
  )
);

DefineFunction(
  array(
    'name'   => "getAliases",
    'desc'   => "Returns valid aliases of the named encoding",
    'flags'  => HasDocComment|IsStatic,
    'return' => array(
      'type'   => StringVec,
      'desc'   => "Valid aliases of the named encoding",
    ),
    'args'   => array(
      array(
        'name'   => "encoding",
        'type'   => String,
        'desc'   => "Encoding to get aliases of",
      ),
    ),
  )
);

DefineFunction(
  array(
    'name'   => "getStandards",
    'desc'   => "Returns list supported standards",
    'flags'  => HasDocComment|IsStatic,
    'return' => array(
      'type'   => StringVec,
      'desc'   => "Available standards",
    ),
  )
);

EndClass();
