<?hh

class IntlChar {
  const string UNICODE_VERSION;

  const int CODEPOINT_MIN;
  const int CODEPOINT_MAX;
  const int FOLD_CASE_DEFAULT;
  const int FOLD_CASE_EXCLUDE_SPECIAL_I;

  const int PROPERTY_ALPHABETIC;
  const int PROPERTY_BINARY_START;
  const int PROPERTY_ASCII_HEX_DIGIT;
  const int PROPERTY_BIDI_CONTROL;
  const int PROPERTY_BIDI_MIRRORED;
  const int PROPERTY_DASH;
  const int PROPERTY_DEFAULT_IGNORABLE_CODE_POINT;
  const int PROPERTY_DEPRECATED;
  const int PROPERTY_DIACRITIC;
  const int PROPERTY_EXTENDER;
  const int PROPERTY_FULL_COMPOSITION_EXCLUSION;
  const int PROPERTY_GRAPHEME_BASE;
  const int PROPERTY_GRAPHEME_EXTEND;
  const int PROPERTY_GRAPHEME_LINK;
  const int PROPERTY_HEX_DIGIT;
  const int PROPERTY_HYPHEN;
  const int PROPERTY_ID_CONTINUE;
  const int PROPERTY_ID_START;
  const int PROPERTY_IDEOGRAPHIC;
  const int PROPERTY_IDS_BINARY_OPERATOR;
  const int PROPERTY_IDS_TRINARY_OPERATOR;
  const int PROPERTY_JOIN_CONTROL;
  const int PROPERTY_LOGICAL_ORDER_EXCEPTION;
  const int PROPERTY_LOWERCASE;
  const int PROPERTY_MATH;
  const int PROPERTY_NONCHARACTER_CODE_POINT;
  const int PROPERTY_QUOTATION_MARK;
  const int PROPERTY_RADICAL;
  const int PROPERTY_SOFT_DOTTED;
  const int PROPERTY_TERMINAL_PUNCTUATION;
  const int PROPERTY_UNIFIED_IDEOGRAPH;
  const int PROPERTY_UPPERCASE;
  const int PROPERTY_WHITE_SPACE;
  const int PROPERTY_XID_CONTINUE;
  const int PROPERTY_XID_START;
  const int PROPERTY_CASE_SENSITIVE;
  const int PROPERTY_S_TERM;
  const int PROPERTY_VARIATION_SELECTOR;
  const int PROPERTY_NFD_INERT;
  const int PROPERTY_NFKD_INERT;
  const int PROPERTY_NFC_INERT;
  const int PROPERTY_NFKC_INERT;
  const int PROPERTY_SEGMENT_STARTER;
  const int PROPERTY_PATTERN_SYNTAX;
  const int PROPERTY_PATTERN_WHITE_SPACE;
  const int PROPERTY_POSIX_ALNUM;
  const int PROPERTY_POSIX_BLANK;
  const int PROPERTY_POSIX_GRAPH;
  const int PROPERTY_POSIX_PRINT;
  const int PROPERTY_POSIX_XDIGIT;
  const int PROPERTY_CASED;
  const int PROPERTY_CASE_IGNORABLE;
  const int PROPERTY_CHANGES_WHEN_LOWERCASED;
  const int PROPERTY_CHANGES_WHEN_UPPERCASED;
  const int PROPERTY_CHANGES_WHEN_TITLECASED;
  const int PROPERTY_CHANGES_WHEN_CASEFOLDED;
  const int PROPERTY_CHANGES_WHEN_CASEMAPPED;
  const int PROPERTY_CHANGES_WHEN_NFKC_CASEFOLDED;

  const int PROPERTY_BINARY_LIMIT;
  const int PROPERTY_BIDI_CLASS;
  const int PROPERTY_INT_START;
  const int PROPERTY_BLOCK;
  const int PROPERTY_CANONICAL_COMBINING_CLASS;
  const int PROPERTY_DECOMPOSITION_TYPE;
  const int PROPERTY_EAST_ASIAN_WIDTH;
  const int PROPERTY_GENERAL_CATEGORY;
  const int PROPERTY_JOINING_GROUP;
  const int PROPERTY_JOINING_TYPE;
  const int PROPERTY_LINE_BREAK;
  const int PROPERTY_NUMERIC_TYPE;
  const int PROPERTY_SCRIPT;
  const int PROPERTY_HANGUL_SYLLABLE_TYPE;
  const int PROPERTY_NFD_QUICK_CHECK;
  const int PROPERTY_NFKD_QUICK_CHECK;
  const int PROPERTY_NFC_QUICK_CHECK;
  const int PROPERTY_NFKC_QUICK_CHECK;
  const int PROPERTY_LEAD_CANONICAL_COMBINING_CLASS;
  const int PROPERTY_TRAIL_CANONICAL_COMBINING_CLASS;
  const int PROPERTY_GRAPHEME_CLUSTER_BREAK;
  const int PROPERTY_SENTENCE_BREAK;
  const int PROPERTY_WORD_BREAK;
  const int PROPERTY_BIDI_PAIRED_BRACKET_TYPE;
  const int PROPERTY_INT_LIMIT;
  const int PROPERTY_GENERAL_CATEGORY_MASK;
  const int PROPERTY_MASK_START;
  const int PROPERTY_MASK_LIMIT;
  const int PROPERTY_NUMERIC_VALUE;
  const int PROPERTY_DOUBLE_START;
  const int PROPERTY_DOUBLE_LIMIT;
  const int PROPERTY_AGE;
  const int PROPERTY_STRING_START;
  const int PROPERTY_BIDI_MIRRORING_GLYPH;
  const int PROPERTY_CASE_FOLDING;
  const int PROPERTY_ISO_COMMENT;
  const int PROPERTY_LOWERCASE_MAPPING;
  const int PROPERTY_NAME;
  const int PROPERTY_SIMPLE_CASE_FOLDING;
  const int PROPERTY_SIMPLE_LOWERCASE_MAPPING;
  const int PROPERTY_SIMPLE_TITLECASE_MAPPING;
  const int PROPERTY_SIMPLE_UPPERCASE_MAPPING;
  const int PROPERTY_TITLECASE_MAPPING;
  const int PROPERTY_UNICODE_1_NAME;
  const int PROPERTY_UPPERCASE_MAPPING;
  const int PROPERTY_BIDI_PAIRED_BRACKET;
  const int PROPERTY_STRING_LIMIT;
  const int PROPERTY_SCRIPT_EXTENSIONS;
  const int PROPERTY_OTHER_PROPERTY_START;
  const int PROPERTY_OTHER_PROPERTY_LIMIT;
  const int PROPERTY_INVALID_CODE;

  const int CHAR_CATEGORY_UNASSIGNED;
  const int CHAR_CATEGORY_GENERAL_OTHER_TYPES;
  const int CHAR_CATEGORY_UPPERCASE_LETTER;
  const int CHAR_CATEGORY_LOWERCASE_LETTER;
  const int CHAR_CATEGORY_TITLECASE_LETTER;
  const int CHAR_CATEGORY_MODIFIER_LETTER;
  const int CHAR_CATEGORY_OTHER_LETTER;
  const int CHAR_CATEGORY_NON_SPACING_MARK;
  const int CHAR_CATEGORY_ENCLOSING_MARK;
  const int CHAR_CATEGORY_COMBINING_SPACING_MARK;
  const int CHAR_CATEGORY_DECIMAL_DIGIT_NUMBER;
  const int CHAR_CATEGORY_LETTER_NUMBER;
  const int CHAR_CATEGORY_OTHER_NUMBER;
  const int CHAR_CATEGORY_SPACE_SEPARATOR;
  const int CHAR_CATEGORY_LINE_SEPARATOR;
  const int CHAR_CATEGORY_PARAGRAPH_SEPARATOR;
  const int CHAR_CATEGORY_CONTROL_CHAR;
  const int CHAR_CATEGORY_FORMAT_CHAR;
  const int CHAR_CATEGORY_PRIVATE_USE_CHAR;
  const int CHAR_CATEGORY_SURROGATE;
  const int CHAR_CATEGORY_DASH_PUNCTUATION;
  const int CHAR_CATEGORY_START_PUNCTUATION;
  const int CHAR_CATEGORY_END_PUNCTUATION;
  const int CHAR_CATEGORY_CONNECTOR_PUNCTUATION;
  const int CHAR_CATEGORY_OTHER_PUNCTUATION;
  const int CHAR_CATEGORY_MATH_SYMBOL;
  const int CHAR_CATEGORY_CURRENCY_SYMBOL;
  const int CHAR_CATEGORY_MODIFIER_SYMBOL;
  const int CHAR_CATEGORY_OTHER_SYMBOL;
  const int CHAR_CATEGORY_INITIAL_PUNCTUATION;
  const int CHAR_CATEGORY_FINAL_PUNCTUATION;
  const int CHAR_CATEGORY_CHAR_CATEGORY_COUNT;
  const int CHAR_DIRECTION_LEFT_TO_RIGHT;
  const int CHAR_DIRECTION_RIGHT_TO_LEFT;
  const int CHAR_DIRECTION_EUROPEAN_NUMBER;
  const int CHAR_DIRECTION_EUROPEAN_NUMBER_SEPARATOR;
  const int CHAR_DIRECTION_EUROPEAN_NUMBER_TERMINATOR;
  const int CHAR_DIRECTION_ARABIC_NUMBER;
  const int CHAR_DIRECTION_COMMON_NUMBER_SEPARATOR;
  const int CHAR_DIRECTION_BLOCK_SEPARATOR;
  const int CHAR_DIRECTION_SEGMENT_SEPARATOR;
  const int CHAR_DIRECTION_WHITE_SPACE_NEUTRAL;
  const int CHAR_DIRECTION_OTHER_NEUTRAL;
  const int CHAR_DIRECTION_LEFT_TO_RIGHT_EMBEDDING;
  const int CHAR_DIRECTION_LEFT_TO_RIGHT_OVERRIDE;
  const int CHAR_DIRECTION_RIGHT_TO_LEFT_ARABIC;
  const int CHAR_DIRECTION_RIGHT_TO_LEFT_EMBEDDING;
  const int CHAR_DIRECTION_RIGHT_TO_LEFT_OVERRIDE;
  const int CHAR_DIRECTION_POP_DIRECTIONAL_FORMAT;
  const int CHAR_DIRECTION_DIR_NON_SPACING_MARK;
  const int CHAR_DIRECTION_BOUNDARY_NEUTRAL;
  const int CHAR_DIRECTION_FIRST_STRONG_ISOLATE;
  const int CHAR_DIRECTION_LEFT_TO_RIGHT_ISOLATE;
  const int CHAR_DIRECTION_RIGHT_TO_LEFT_ISOLATE;
  const int CHAR_DIRECTION_POP_DIRECTIONAL_ISOLATE;
  const int CHAR_DIRECTION_CHAR_DIRECTION_COUNT;

  const int BLOCK_CODE_NO_BLOCK;
  const int BLOCK_CODE_BASIC_LATIN;
  const int BLOCK_CODE_LATIN_1_SUPPLEMENT;
  const int BLOCK_CODE_LATIN_EXTENDED_A;
  const int BLOCK_CODE_LATIN_EXTENDED_B;
  const int BLOCK_CODE_IPA_EXTENSIONS;
  const int BLOCK_CODE_SPACING_MODIFIER_LETTERS;
  const int BLOCK_CODE_COMBINING_DIACRITICAL_MARKS;
  const int BLOCK_CODE_GREEK;
  const int BLOCK_CODE_CYRILLIC;
  const int BLOCK_CODE_ARMENIAN;
  const int BLOCK_CODE_HEBREW;
  const int BLOCK_CODE_ARABIC;
  const int BLOCK_CODE_SYRIAC;
  const int BLOCK_CODE_THAANA;
  const int BLOCK_CODE_DEVANAGARI;
  const int BLOCK_CODE_BENGALI;
  const int BLOCK_CODE_GURMUKHI;
  const int BLOCK_CODE_GUJARATI;
  const int BLOCK_CODE_ORIYA;
  const int BLOCK_CODE_TAMIL;
  const int BLOCK_CODE_TELUGU;
  const int BLOCK_CODE_KANNADA;
  const int BLOCK_CODE_MALAYALAM;
  const int BLOCK_CODE_SINHALA;
  const int BLOCK_CODE_THAI;
  const int BLOCK_CODE_LAO;
  const int BLOCK_CODE_TIBETAN;
  const int BLOCK_CODE_MYANMAR;
  const int BLOCK_CODE_GEORGIAN;
  const int BLOCK_CODE_HANGUL_JAMO;
  const int BLOCK_CODE_ETHIOPIC;
  const int BLOCK_CODE_CHEROKEE;
  const int BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS;
  const int BLOCK_CODE_OGHAM;
  const int BLOCK_CODE_RUNIC;
  const int BLOCK_CODE_KHMER;
  const int BLOCK_CODE_MONGOLIAN;
  const int BLOCK_CODE_LATIN_EXTENDED_ADDITIONAL;
  const int BLOCK_CODE_GREEK_EXTENDED;
  const int BLOCK_CODE_GENERAL_PUNCTUATION;
  const int BLOCK_CODE_SUPERSCRIPTS_AND_SUBSCRIPTS;
  const int BLOCK_CODE_CURRENCY_SYMBOLS;
  const int BLOCK_CODE_COMBINING_MARKS_FOR_SYMBOLS;
  const int BLOCK_CODE_LETTERLIKE_SYMBOLS;
  const int BLOCK_CODE_NUMBER_FORMS;
  const int BLOCK_CODE_ARROWS;
  const int BLOCK_CODE_MATHEMATICAL_OPERATORS;
  const int BLOCK_CODE_MISCELLANEOUS_TECHNICAL;
  const int BLOCK_CODE_CONTROL_PICTURES;
  const int BLOCK_CODE_OPTICAL_CHARACTER_RECOGNITION;
  const int BLOCK_CODE_ENCLOSED_ALPHANUMERICS;
  const int BLOCK_CODE_BOX_DRAWING;
  const int BLOCK_CODE_BLOCK_ELEMENTS;
  const int BLOCK_CODE_GEOMETRIC_SHAPES;
  const int BLOCK_CODE_MISCELLANEOUS_SYMBOLS;
  const int BLOCK_CODE_DINGBATS;
  const int BLOCK_CODE_BRAILLE_PATTERNS;
  const int BLOCK_CODE_CJK_RADICALS_SUPPLEMENT;
  const int BLOCK_CODE_KANGXI_RADICALS;
  const int BLOCK_CODE_IDEOGRAPHIC_DESCRIPTION_CHARACTERS;
  const int BLOCK_CODE_CJK_SYMBOLS_AND_PUNCTUATION;
  const int BLOCK_CODE_HIRAGANA;
  const int BLOCK_CODE_KATAKANA;
  const int BLOCK_CODE_BOPOMOFO;
  const int BLOCK_CODE_HANGUL_COMPATIBILITY_JAMO;
  const int BLOCK_CODE_KANBUN;
  const int BLOCK_CODE_BOPOMOFO_EXTENDED;
  const int BLOCK_CODE_ENCLOSED_CJK_LETTERS_AND_MONTHS;
  const int BLOCK_CODE_CJK_COMPATIBILITY;
  const int BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A;
  const int BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS;
  const int BLOCK_CODE_YI_SYLLABLES;
  const int BLOCK_CODE_YI_RADICALS;
  const int BLOCK_CODE_HANGUL_SYLLABLES;
  const int BLOCK_CODE_HIGH_SURROGATES;
  const int BLOCK_CODE_HIGH_PRIVATE_USE_SURROGATES;
  const int BLOCK_CODE_LOW_SURROGATES;
  const int BLOCK_CODE_PRIVATE_USE_AREA;
  const int BLOCK_CODE_PRIVATE_USE;
  const int BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS;
  const int BLOCK_CODE_ALPHABETIC_PRESENTATION_FORMS;
  const int BLOCK_CODE_ARABIC_PRESENTATION_FORMS_A;
  const int BLOCK_CODE_COMBINING_HALF_MARKS;
  const int BLOCK_CODE_CJK_COMPATIBILITY_FORMS;
  const int BLOCK_CODE_SMALL_FORM_VARIANTS;
  const int BLOCK_CODE_ARABIC_PRESENTATION_FORMS_B;
  const int BLOCK_CODE_SPECIALS;
  const int BLOCK_CODE_HALFWIDTH_AND_FULLWIDTH_FORMS;
  const int BLOCK_CODE_OLD_ITALIC;
  const int BLOCK_CODE_GOTHIC;
  const int BLOCK_CODE_DESERET;
  const int BLOCK_CODE_BYZANTINE_MUSICAL_SYMBOLS;
  const int BLOCK_CODE_MUSICAL_SYMBOLS;
  const int BLOCK_CODE_MATHEMATICAL_ALPHANUMERIC_SYMBOLS;
  const int BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B;
  const int BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT;
  const int BLOCK_CODE_TAGS;
  const int BLOCK_CODE_CYRILLIC_SUPPLEMENT;
  const int BLOCK_CODE_CYRILLIC_SUPPLEMENTARY;
  const int BLOCK_CODE_TAGALOG;
  const int BLOCK_CODE_HANUNOO;
  const int BLOCK_CODE_BUHID;
  const int BLOCK_CODE_TAGBANWA;
  const int BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A;
  const int BLOCK_CODE_SUPPLEMENTAL_ARROWS_A;
  const int BLOCK_CODE_SUPPLEMENTAL_ARROWS_B;
  const int BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B;
  const int BLOCK_CODE_SUPPLEMENTAL_MATHEMATICAL_OPERATORS;
  const int BLOCK_CODE_KATAKANA_PHONETIC_EXTENSIONS;
  const int BLOCK_CODE_VARIATION_SELECTORS;
  const int BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_A;
  const int BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_B;
  const int BLOCK_CODE_LIMBU;
  const int BLOCK_CODE_TAI_LE;
  const int BLOCK_CODE_KHMER_SYMBOLS;
  const int BLOCK_CODE_PHONETIC_EXTENSIONS;
  const int BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_ARROWS;
  const int BLOCK_CODE_YIJING_HEXAGRAM_SYMBOLS;
  const int BLOCK_CODE_LINEAR_B_SYLLABARY;
  const int BLOCK_CODE_LINEAR_B_IDEOGRAMS;
  const int BLOCK_CODE_AEGEAN_NUMBERS;
  const int BLOCK_CODE_UGARITIC;
  const int BLOCK_CODE_SHAVIAN;
  const int BLOCK_CODE_OSMANYA;
  const int BLOCK_CODE_CYPRIOT_SYLLABARY;
  const int BLOCK_CODE_TAI_XUAN_JING_SYMBOLS;
  const int BLOCK_CODE_VARIATION_SELECTORS_SUPPLEMENT;
  const int BLOCK_CODE_ANCIENT_GREEK_MUSICAL_NOTATION;
  const int BLOCK_CODE_ANCIENT_GREEK_NUMBERS;
  const int BLOCK_CODE_ARABIC_SUPPLEMENT;
  const int BLOCK_CODE_BUGINESE;
  const int BLOCK_CODE_CJK_STROKES;
  const int BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT;
  const int BLOCK_CODE_COPTIC;
  const int BLOCK_CODE_ETHIOPIC_EXTENDED;
  const int BLOCK_CODE_ETHIOPIC_SUPPLEMENT;
  const int BLOCK_CODE_GEORGIAN_SUPPLEMENT;
  const int BLOCK_CODE_GLAGOLITIC;
  const int BLOCK_CODE_KHAROSHTHI;
  const int BLOCK_CODE_MODIFIER_TONE_LETTERS;
  const int BLOCK_CODE_NEW_TAI_LUE;
  const int BLOCK_CODE_OLD_PERSIAN;
  const int BLOCK_CODE_PHONETIC_EXTENSIONS_SUPPLEMENT;
  const int BLOCK_CODE_SUPPLEMENTAL_PUNCTUATION;
  const int BLOCK_CODE_SYLOTI_NAGRI;
  const int BLOCK_CODE_TIFINAGH;
  const int BLOCK_CODE_VERTICAL_FORMS;
  const int BLOCK_CODE_NKO;
  const int BLOCK_CODE_BALINESE;
  const int BLOCK_CODE_LATIN_EXTENDED_C;
  const int BLOCK_CODE_LATIN_EXTENDED_D;
  const int BLOCK_CODE_PHAGS_PA;
  const int BLOCK_CODE_PHOENICIAN;
  const int BLOCK_CODE_CUNEIFORM;
  const int BLOCK_CODE_CUNEIFORM_NUMBERS_AND_PUNCTUATION;
  const int BLOCK_CODE_COUNTING_ROD_NUMERALS;
  const int BLOCK_CODE_SUNDANESE;
  const int BLOCK_CODE_LEPCHA;
  const int BLOCK_CODE_OL_CHIKI;
  const int BLOCK_CODE_CYRILLIC_EXTENDED_A;
  const int BLOCK_CODE_VAI;
  const int BLOCK_CODE_CYRILLIC_EXTENDED_B;
  const int BLOCK_CODE_SAURASHTRA;
  const int BLOCK_CODE_KAYAH_LI;
  const int BLOCK_CODE_REJANG;
  const int BLOCK_CODE_CHAM;
  const int BLOCK_CODE_ANCIENT_SYMBOLS;
  const int BLOCK_CODE_PHAISTOS_DISC;
  const int BLOCK_CODE_LYCIAN;
  const int BLOCK_CODE_CARIAN;
  const int BLOCK_CODE_LYDIAN;
  const int BLOCK_CODE_MAHJONG_TILES;
  const int BLOCK_CODE_DOMINO_TILES;
  const int BLOCK_CODE_SAMARITAN;
  const int BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED;
  const int BLOCK_CODE_TAI_THAM;
  const int BLOCK_CODE_VEDIC_EXTENSIONS;
  const int BLOCK_CODE_LISU;
  const int BLOCK_CODE_BAMUM;
  const int BLOCK_CODE_COMMON_INDIC_NUMBER_FORMS;
  const int BLOCK_CODE_DEVANAGARI_EXTENDED;
  const int BLOCK_CODE_HANGUL_JAMO_EXTENDED_A;
  const int BLOCK_CODE_JAVANESE;
  const int BLOCK_CODE_MYANMAR_EXTENDED_A;
  const int BLOCK_CODE_TAI_VIET;
  const int BLOCK_CODE_MEETEI_MAYEK;
  const int BLOCK_CODE_HANGUL_JAMO_EXTENDED_B;
  const int BLOCK_CODE_IMPERIAL_ARAMAIC;
  const int BLOCK_CODE_OLD_SOUTH_ARABIAN;
  const int BLOCK_CODE_AVESTAN;
  const int BLOCK_CODE_INSCRIPTIONAL_PARTHIAN;
  const int BLOCK_CODE_INSCRIPTIONAL_PAHLAVI;
  const int BLOCK_CODE_OLD_TURKIC;
  const int BLOCK_CODE_RUMI_NUMERAL_SYMBOLS;
  const int BLOCK_CODE_KAITHI;
  const int BLOCK_CODE_EGYPTIAN_HIEROGLYPHS;
  const int BLOCK_CODE_ENCLOSED_ALPHANUMERIC_SUPPLEMENT;
  const int BLOCK_CODE_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT;
  const int BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C;
  const int BLOCK_CODE_MANDAIC;
  const int BLOCK_CODE_BATAK;
  const int BLOCK_CODE_ETHIOPIC_EXTENDED_A;
  const int BLOCK_CODE_BRAHMI;
  const int BLOCK_CODE_BAMUM_SUPPLEMENT;
  const int BLOCK_CODE_KANA_SUPPLEMENT;
  const int BLOCK_CODE_PLAYING_CARDS;
  const int BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS;
  const int BLOCK_CODE_EMOTICONS;
  const int BLOCK_CODE_TRANSPORT_AND_MAP_SYMBOLS;
  const int BLOCK_CODE_ALCHEMICAL_SYMBOLS;
  const int BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D;
  const int BLOCK_CODE_ARABIC_EXTENDED_A;
  const int BLOCK_CODE_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS;
  const int BLOCK_CODE_CHAKMA;
  const int BLOCK_CODE_MEETEI_MAYEK_EXTENSIONS;
  const int BLOCK_CODE_MEROITIC_CURSIVE;
  const int BLOCK_CODE_MEROITIC_HIEROGLYPHS;
  const int BLOCK_CODE_MIAO;
  const int BLOCK_CODE_SHARADA;
  const int BLOCK_CODE_SORA_SOMPENG;
  const int BLOCK_CODE_SUNDANESE_SUPPLEMENT;
  const int BLOCK_CODE_TAKRI;
  const int BLOCK_CODE_COUNT;
  const int BLOCK_CODE_INVALID_CODE;

  const int EA_NEUTRAL;
  const int EA_AMBIGUOUS;
  const int EA_HALFWIDTH;
  const int EA_FULLWIDTH;
  const int EA_NARROW;
  const int EA_WIDE;
  const int EA_COUNT;

  const int UNICODE_CHAR_NAME;
  const int UNICODE_10_CHAR_NAME;
  const int EXTENDED_CHAR_NAME;

  const int CHAR_NAME_CHOICE_COUNT;

  const int SHORT_PROPERTY_NAME;
  const int LONG_PROPERTY_NAME;
  const int PROPERTY_NAME_CHOICE_COUNT;

  const int DT_NONE;
  const int DT_CANONICAL;
  const int DT_COMPAT;
  const int DT_CIRCLE;
  const int DT_FINAL;
  const int DT_FONT;
  const int DT_FRACTION;
  const int DT_INITIAL;
  const int DT_ISOLATED;
  const int DT_MEDIAL;
  const int DT_NARROW;
  const int DT_NOBREAK;
  const int DT_SMALL;
  const int DT_SQUARE;
  const int DT_SUB;
  const int DT_SUPER;
  const int DT_VERTICAL;
  const int DT_WIDE;
  const int DT_COUNT;

  const int JT_NON_JOINING;
  const int JT_JOIN_CAUSING;
  const int JT_DUAL_JOINING;
  const int JT_LEFT_JOINING;
  const int JT_RIGHT_JOINING;
  const int JT_TRANSPARENT;
  const int JT_COUNT;

  const int JG_NO_JOINING_GROUP;
  const int JG_AIN;
  const int JG_ALAPH;
  const int JG_ALEF;
  const int JG_BEH;
  const int JG_BETH;
  const int JG_DAL;
  const int JG_DALATH_RISH;
  const int JG_E;
  const int JG_FEH;
  const int JG_FINAL_SEMKATH;
  const int JG_GAF;
  const int JG_GAMAL;
  const int JG_HAH;

  const int JG_HAMZA_ON_HEH_GOAL;
  const int JG_HE;
  const int JG_HEH;
  const int JG_HEH_GOAL;
  const int JG_HETH;
  const int JG_KAF;
  const int JG_KAPH;
  const int JG_KNOTTED_HEH;
  const int JG_LAM;
  const int JG_LAMADH;
  const int JG_MEEM;
  const int JG_MIM;
  const int JG_NOON;
  const int JG_NUN;
  const int JG_PE;
  const int JG_QAF;
  const int JG_QAPH;
  const int JG_REH;
  const int JG_REVERSED_PE;
  const int JG_SAD;
  const int JG_SADHE;
  const int JG_SEEN;
  const int JG_SEMKATH;
  const int JG_SHIN;
  const int JG_SWASH_KAF;
  const int JG_SYRIAC_WAW;
  const int JG_TAH;
  const int JG_TAW;
  const int JG_TEH_MARBUTA;
  const int JG_TETH;
  const int JG_WAW;
  const int JG_YEH;
  const int JG_YEH_BARREE;
  const int JG_YEH_WITH_TAIL;
  const int JG_YUDH;
  const int JG_YUDH_HE;
  const int JG_ZAIN;
  const int JG_FE;
  const int JG_KHAPH;
  const int JG_ZHAIN;
  const int JG_BURUSHASKI_YEH_BARREE;
  const int JG_COUNT;

  const int GCB_OTHER;
  const int GCB_CONTROL;
  const int GCB_CR;
  const int GCB_EXTEND;
  const int GCB_L;
  const int GCB_LF;
  const int GCB_LV;
  const int GCB_LVT;
  const int GCB_T;
  const int GCB_V;
  const int GCB_SPACING_MARK;
  const int GCB_PREPEND;
  const int GCB_COUNT;

  const int WB_OTHER;
  const int WB_ALETTER;
  const int WB_FORMAT;
  const int WB_KATAKANA;
  const int WB_MIDLETTER;
  const int WB_MIDNUM;
  const int WB_NUMERIC;
  const int WB_EXTENDNUMLET;
  const int WB_CR;
  const int WB_EXTEND;
  const int WB_LF;
  const int WB_MIDNUMLET;
  const int WB_NEWLINE;
  const int WB_COUNT;

  const int SB_OTHER;
  const int SB_ATERM;
  const int SB_CLOSE;
  const int SB_FORMAT;
  const int SB_LOWER;
  const int SB_NUMERIC;
  const int SB_OLETTER;
  const int SB_SEP;
  const int SB_SP;
  const int SB_STERM;
  const int SB_UPPER;
  const int SB_CR;
  const int SB_EXTEND;
  const int SB_LF;
  const int SB_SCONTINUE;
  const int SB_COUNT;

  const int LB_UNKNOWN;
  const int LB_AMBIGUOUS;
  const int LB_ALPHABETIC;
  const int LB_BREAK_BOTH;
  const int LB_BREAK_AFTER;
  const int LB_BREAK_BEFORE;
  const int LB_MANDATORY_BREAK;
  const int LB_CONTINGENT_BREAK;
  const int LB_CLOSE_PUNCTUATION;
  const int LB_COMBINING_MARK;
  const int LB_CARRIAGE_RETURN;
  const int LB_EXCLAMATION;
  const int LB_GLUE;
  const int LB_HYPHEN;
  const int LB_IDEOGRAPHIC;
  const int LB_INSEPARABLE;
  const int LB_INSEPERABLE;
  const int LB_INFIX_NUMERIC;
  const int LB_LINE_FEED;
  const int LB_NONSTARTER;
  const int LB_NUMERIC;
  const int LB_OPEN_PUNCTUATION;
  const int LB_POSTFIX_NUMERIC;
  const int LB_PREFIX_NUMERIC;
  const int LB_QUOTATION;
  const int LB_COMPLEX_CONTEXT;
  const int LB_SURROGATE;
  const int LB_SPACE;
  const int LB_BREAK_SYMBOLS;
  const int LB_ZWSPACE;
  const int LB_NEXT_LINE;
  const int LB_WORD_JOINER;
  const int LB_H2;
  const int LB_H3;
  const int LB_JL;
  const int LB_JT;
  const int LB_JV;
  const int LB_COUNT;

  const int NT_NONE;
  const int NT_DECIMAL;
  const int NT_DIGIT;
  const int NT_NUMERIC;
  const int NT_COUNT;

  const int HST_NOT_APPLICABLE;
  const int HST_LEADING_JAMO;
  const int HST_VOWEL_JAMO;
  const int HST_TRAILING_JAMO;
  const int HST_LV_SYLLABLE;
  const int HST_LVT_SYLLABLE;
  const int HST_COUNT;

  static public function chr(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function ord(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function hasBinaryProperty(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
    int $prop,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function getIntPropertyValue(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
    int $prop,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function getIntPropertyMaxValue(int $prop): int {
    return 0;
  }
  static public function getIntPropertyMinValue(int $prop): int {
    return 0;
  }
  static public function getNumericValue(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function enumCharTypes(
    (function(int, int, int): void) $cb,
  ): void {}
  static public function getBlockCode(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function charName(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
    int $choice = IntlChar::UNICODE_CHAR_NAME,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function charFromName(
    string $name,
    int $choice = IntlChar::UNICODE_CHAR_NAME,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function enumCharNames(
    HH\FIXME\MISSING_PARAM_TYPE $start,
    HH\FIXME\MISSING_PARAM_TYPE $limit,
    (function(int, int, string): void) $cb,
    int $choice = IntlChar::UNICODE_CHAR_NAME,
  ): void {}
  static public function getPropertyName(
    int $prop,
    int $choice = IntlChar::LONG_PROPERTY_NAME,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function getPropertyValueName(
    int $prop,
    int $value,
    int $choice = IntlChar::LONG_PROPERTY_NAME,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function getPropertyEnum(string $alias): int {
    return 0;
  }
  static public function getPropertyValueEnum(int $prop, string $name): int {
    return 0;
  }
  static public function foldCase(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
    int $options = IntlChar::FOLD_CASE_DEFAULT,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function digit(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
    int $radix = 10,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function forDigit(int $digit, int $radix = 10): int {
    return 0;
  }
  static public function charAge(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  <<__PHPStdLib>>
  static public function getUnicodeVersion(): darray<arraykey, mixed> {
    return dict[];
  }
  static public function getFC_NFKC_Closure(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isUAlphabetic(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isULowercase(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isUUppercase(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isUWhiteSpace(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function islower(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isupper(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function istitle(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isdigit(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isalpha(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isalnum(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isxdigit(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function ispunct(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isgraph(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isblank(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isdefined(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isspace(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isJavaSpaceChar(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isWhitespace(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function iscntrl(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isISOControl(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isprint(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isbase(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isMirrored(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isIDStart(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isIDPart(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isIDIgnorable(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isJavaIDStart(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function isJavaIDPart(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function charDirection(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function charType(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function getCombiningClass(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function charDigitValue(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function getBidiPairedBracket(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function charMirror(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function tolower(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function toupper(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
  static public function totitle(
    HH\FIXME\MISSING_PARAM_TYPE $cp,
  ): HH\FIXME\MISSING_RETURN_TYPE {}
}
