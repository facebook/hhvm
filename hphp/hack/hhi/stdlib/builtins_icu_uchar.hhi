<?hh // decl

class IntlChar {
  const string UNICODE_VERSION = '';

  const int CODEPOINT_MIN = 0;
  const int CODEPOINT_MAX = 0;
  const int FOLD_CASE_DEFAULT = 0;
  const int FOLD_CASE_EXCLUDE_SPECIAL_I = 0;

  const int PROPERTY_ALPHABETIC = 0;
  const int PROPERTY_BINARY_START = 0;
  const int PROPERTY_ASCII_HEX_DIGIT = 0;
  const int PROPERTY_BIDI_CONTROL = 0;
  const int PROPERTY_BIDI_MIRRORED = 0;
  const int PROPERTY_DASH = 0;
  const int PROPERTY_DEFAULT_IGNORABLE_CODE_POINT = 0;
  const int PROPERTY_DEPRECATED = 0;
  const int PROPERTY_DIACRITIC = 0;
  const int PROPERTY_EXTENDER = 0;
  const int PROPERTY_FULL_COMPOSITION_EXCLUSION = 0;
  const int PROPERTY_GRAPHEME_BASE = 0;
  const int PROPERTY_GRAPHEME_EXTEND = 0;
  const int PROPERTY_GRAPHEME_LINK = 0;
  const int PROPERTY_HEX_DIGIT = 0;
  const int PROPERTY_HYPHEN = 0;
  const int PROPERTY_ID_CONTINUE = 0;
  const int PROPERTY_ID_START = 0;
  const int PROPERTY_IDEOGRAPHIC = 0;
  const int PROPERTY_IDS_BINARY_OPERATOR = 0;
  const int PROPERTY_IDS_TRINARY_OPERATOR = 0;
  const int PROPERTY_JOIN_CONTROL = 0;
  const int PROPERTY_LOGICAL_ORDER_EXCEPTION = 0;
  const int PROPERTY_LOWERCASE = 0;
  const int PROPERTY_MATH = 0;
  const int PROPERTY_NONCHARACTER_CODE_POINT = 0;
  const int PROPERTY_QUOTATION_MARK = 0;
  const int PROPERTY_RADICAL = 0;
  const int PROPERTY_SOFT_DOTTED = 0;
  const int PROPERTY_TERMINAL_PUNCTUATION = 0;
  const int PROPERTY_UNIFIED_IDEOGRAPH = 0;
  const int PROPERTY_UPPERCASE = 0;
  const int PROPERTY_WHITE_SPACE = 0;
  const int PROPERTY_XID_CONTINUE = 0;
  const int PROPERTY_XID_START = 0;
  const int PROPERTY_CASE_SENSITIVE = 0;
  const int PROPERTY_S_TERM = 0;
  const int PROPERTY_VARIATION_SELECTOR = 0;
  const int PROPERTY_NFD_INERT = 0;
  const int PROPERTY_NFKD_INERT = 0;
  const int PROPERTY_NFC_INERT = 0;
  const int PROPERTY_NFKC_INERT = 0;
  const int PROPERTY_SEGMENT_STARTER = 0;
  const int PROPERTY_PATTERN_SYNTAX = 0;
  const int PROPERTY_PATTERN_WHITE_SPACE = 0;
  const int PROPERTY_POSIX_ALNUM = 0;
  const int PROPERTY_POSIX_BLANK = 0;
  const int PROPERTY_POSIX_GRAPH = 0;
  const int PROPERTY_POSIX_PRINT = 0;
  const int PROPERTY_POSIX_XDIGIT = 0;
  const int PROPERTY_CASED = 0;
  const int PROPERTY_CASE_IGNORABLE = 0;
  const int PROPERTY_CHANGES_WHEN_LOWERCASED = 0;
  const int PROPERTY_CHANGES_WHEN_UPPERCASED = 0;
  const int PROPERTY_CHANGES_WHEN_TITLECASED = 0;
  const int PROPERTY_CHANGES_WHEN_CASEFOLDED = 0;
  const int PROPERTY_CHANGES_WHEN_CASEMAPPED = 0;
  const int PROPERTY_CHANGES_WHEN_NFKC_CASEFOLDED = 0;

  const int PROPERTY_BINARY_LIMIT = 0;
  const int PROPERTY_BIDI_CLASS = 0;
  const int PROPERTY_INT_START = 0;
  const int PROPERTY_BLOCK = 0;
  const int PROPERTY_CANONICAL_COMBINING_CLASS = 0;
  const int PROPERTY_DECOMPOSITION_TYPE = 0;
  const int PROPERTY_EAST_ASIAN_WIDTH = 0;
  const int PROPERTY_GENERAL_CATEGORY = 0;
  const int PROPERTY_JOINING_GROUP = 0;
  const int PROPERTY_JOINING_TYPE = 0;
  const int PROPERTY_LINE_BREAK = 0;
  const int PROPERTY_NUMERIC_TYPE = 0;
  const int PROPERTY_SCRIPT = 0;
  const int PROPERTY_HANGUL_SYLLABLE_TYPE = 0;
  const int PROPERTY_NFD_QUICK_CHECK = 0;
  const int PROPERTY_NFKD_QUICK_CHECK = 0;
  const int PROPERTY_NFC_QUICK_CHECK = 0;
  const int PROPERTY_NFKC_QUICK_CHECK = 0;
  const int PROPERTY_LEAD_CANONICAL_COMBINING_CLASS = 0;
  const int PROPERTY_TRAIL_CANONICAL_COMBINING_CLASS = 0;
  const int PROPERTY_GRAPHEME_CLUSTER_BREAK = 0;
  const int PROPERTY_SENTENCE_BREAK = 0;
  const int PROPERTY_WORD_BREAK = 0;
  const int PROPERTY_BIDI_PAIRED_BRACKET_TYPE = 0;
  const int PROPERTY_INT_LIMIT = 0;
  const int PROPERTY_GENERAL_CATEGORY_MASK = 0;
  const int PROPERTY_MASK_START = 0;
  const int PROPERTY_MASK_LIMIT = 0;
  const int PROPERTY_NUMERIC_VALUE = 0;
  const int PROPERTY_DOUBLE_START = 0;
  const int PROPERTY_DOUBLE_LIMIT = 0;
  const int PROPERTY_AGE = 0;
  const int PROPERTY_STRING_START = 0;
  const int PROPERTY_BIDI_MIRRORING_GLYPH = 0;
  const int PROPERTY_CASE_FOLDING = 0;
  const int PROPERTY_ISO_COMMENT = 0;
  const int PROPERTY_LOWERCASE_MAPPING = 0;
  const int PROPERTY_NAME = 0;
  const int PROPERTY_SIMPLE_CASE_FOLDING = 0;
  const int PROPERTY_SIMPLE_LOWERCASE_MAPPING = 0;
  const int PROPERTY_SIMPLE_TITLECASE_MAPPING = 0;
  const int PROPERTY_SIMPLE_UPPERCASE_MAPPING = 0;
  const int PROPERTY_TITLECASE_MAPPING = 0;
  const int PROPERTY_UNICODE_1_NAME = 0;
  const int PROPERTY_UPPERCASE_MAPPING = 0;
  const int PROPERTY_BIDI_PAIRED_BRACKET = 0;
  const int PROPERTY_STRING_LIMIT = 0;
  const int PROPERTY_SCRIPT_EXTENSIONS = 0;
  const int PROPERTY_OTHER_PROPERTY_START = 0;
  const int PROPERTY_OTHER_PROPERTY_LIMIT = 0;
  const int PROPERTY_INVALID_CODE = 0;

  const int CHAR_CATEGORY_UNASSIGNED = 0;
  const int CHAR_CATEGORY_GENERAL_OTHER_TYPES = 0;
  const int CHAR_CATEGORY_UPPERCASE_LETTER = 0;
  const int CHAR_CATEGORY_LOWERCASE_LETTER = 0;
  const int CHAR_CATEGORY_TITLECASE_LETTER = 0;
  const int CHAR_CATEGORY_MODIFIER_LETTER = 0;
  const int CHAR_CATEGORY_OTHER_LETTER = 0;
  const int CHAR_CATEGORY_NON_SPACING_MARK = 0;
  const int CHAR_CATEGORY_ENCLOSING_MARK = 0;
  const int CHAR_CATEGORY_COMBINING_SPACING_MARK = 0;
  const int CHAR_CATEGORY_DECIMAL_DIGIT_NUMBER = 0;
  const int CHAR_CATEGORY_LETTER_NUMBER = 0;
  const int CHAR_CATEGORY_OTHER_NUMBER = 0;
  const int CHAR_CATEGORY_SPACE_SEPARATOR = 0;
  const int CHAR_CATEGORY_LINE_SEPARATOR = 0;
  const int CHAR_CATEGORY_PARAGRAPH_SEPARATOR = 0;
  const int CHAR_CATEGORY_CONTROL_CHAR = 0;
  const int CHAR_CATEGORY_FORMAT_CHAR = 0;
  const int CHAR_CATEGORY_PRIVATE_USE_CHAR = 0;
  const int CHAR_CATEGORY_SURROGATE = 0;
  const int CHAR_CATEGORY_DASH_PUNCTUATION = 0;
  const int CHAR_CATEGORY_START_PUNCTUATION = 0;
  const int CHAR_CATEGORY_END_PUNCTUATION = 0;
  const int CHAR_CATEGORY_CONNECTOR_PUNCTUATION = 0;
  const int CHAR_CATEGORY_OTHER_PUNCTUATION = 0;
  const int CHAR_CATEGORY_MATH_SYMBOL = 0;
  const int CHAR_CATEGORY_CURRENCY_SYMBOL = 0;
  const int CHAR_CATEGORY_MODIFIER_SYMBOL = 0;
  const int CHAR_CATEGORY_OTHER_SYMBOL = 0;
  const int CHAR_CATEGORY_INITIAL_PUNCTUATION = 0;
  const int CHAR_CATEGORY_FINAL_PUNCTUATION = 0;
  const int CHAR_CATEGORY_CHAR_CATEGORY_COUNT = 0;
  const int CHAR_DIRECTION_LEFT_TO_RIGHT = 0;
  const int CHAR_DIRECTION_RIGHT_TO_LEFT = 0;
  const int CHAR_DIRECTION_EUROPEAN_NUMBER = 0;
  const int CHAR_DIRECTION_EUROPEAN_NUMBER_SEPARATOR = 0;
  const int CHAR_DIRECTION_EUROPEAN_NUMBER_TERMINATOR = 0;
  const int CHAR_DIRECTION_ARABIC_NUMBER = 0;
  const int CHAR_DIRECTION_COMMON_NUMBER_SEPARATOR = 0;
  const int CHAR_DIRECTION_BLOCK_SEPARATOR = 0;
  const int CHAR_DIRECTION_SEGMENT_SEPARATOR = 0;
  const int CHAR_DIRECTION_WHITE_SPACE_NEUTRAL = 0;
  const int CHAR_DIRECTION_OTHER_NEUTRAL = 0;
  const int CHAR_DIRECTION_LEFT_TO_RIGHT_EMBEDDING = 0;
  const int CHAR_DIRECTION_LEFT_TO_RIGHT_OVERRIDE = 0;
  const int CHAR_DIRECTION_RIGHT_TO_LEFT_ARABIC = 0;
  const int CHAR_DIRECTION_RIGHT_TO_LEFT_EMBEDDING = 0;
  const int CHAR_DIRECTION_RIGHT_TO_LEFT_OVERRIDE = 0;
  const int CHAR_DIRECTION_POP_DIRECTIONAL_FORMAT = 0;
  const int CHAR_DIRECTION_DIR_NON_SPACING_MARK = 0;
  const int CHAR_DIRECTION_BOUNDARY_NEUTRAL = 0;
  const int CHAR_DIRECTION_FIRST_STRONG_ISOLATE = 0;
  const int CHAR_DIRECTION_LEFT_TO_RIGHT_ISOLATE = 0;
  const int CHAR_DIRECTION_RIGHT_TO_LEFT_ISOLATE = 0;
  const int CHAR_DIRECTION_POP_DIRECTIONAL_ISOLATE = 0;
  const int CHAR_DIRECTION_CHAR_DIRECTION_COUNT = 0;

  const int BLOCK_CODE_NO_BLOCK = 0;
  const int BLOCK_CODE_BASIC_LATIN = 0;
  const int BLOCK_CODE_LATIN_1_SUPPLEMENT = 0;
  const int BLOCK_CODE_LATIN_EXTENDED_A = 0;
  const int BLOCK_CODE_LATIN_EXTENDED_B = 0;
  const int BLOCK_CODE_IPA_EXTENSIONS = 0;
  const int BLOCK_CODE_SPACING_MODIFIER_LETTERS = 0;
  const int BLOCK_CODE_COMBINING_DIACRITICAL_MARKS = 0;
  const int BLOCK_CODE_GREEK = 0;
  const int BLOCK_CODE_CYRILLIC = 0;
  const int BLOCK_CODE_ARMENIAN = 0;
  const int BLOCK_CODE_HEBREW = 0;
  const int BLOCK_CODE_ARABIC = 0;
  const int BLOCK_CODE_SYRIAC = 0;
  const int BLOCK_CODE_THAANA = 0;
  const int BLOCK_CODE_DEVANAGARI = 0;
  const int BLOCK_CODE_BENGALI = 0;
  const int BLOCK_CODE_GURMUKHI = 0;
  const int BLOCK_CODE_GUJARATI = 0;
  const int BLOCK_CODE_ORIYA = 0;
  const int BLOCK_CODE_TAMIL = 0;
  const int BLOCK_CODE_TELUGU = 0;
  const int BLOCK_CODE_KANNADA = 0;
  const int BLOCK_CODE_MALAYALAM = 0;
  const int BLOCK_CODE_SINHALA = 0;
  const int BLOCK_CODE_THAI = 0;
  const int BLOCK_CODE_LAO = 0;
  const int BLOCK_CODE_TIBETAN = 0;
  const int BLOCK_CODE_MYANMAR = 0;
  const int BLOCK_CODE_GEORGIAN = 0;
  const int BLOCK_CODE_HANGUL_JAMO = 0;
  const int BLOCK_CODE_ETHIOPIC = 0;
  const int BLOCK_CODE_CHEROKEE = 0;
  const int BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS = 0;
  const int BLOCK_CODE_OGHAM = 0;
  const int BLOCK_CODE_RUNIC = 0;
  const int BLOCK_CODE_KHMER = 0;
  const int BLOCK_CODE_MONGOLIAN = 0;
  const int BLOCK_CODE_LATIN_EXTENDED_ADDITIONAL = 0;
  const int BLOCK_CODE_GREEK_EXTENDED = 0;
  const int BLOCK_CODE_GENERAL_PUNCTUATION = 0;
  const int BLOCK_CODE_SUPERSCRIPTS_AND_SUBSCRIPTS = 0;
  const int BLOCK_CODE_CURRENCY_SYMBOLS = 0;
  const int BLOCK_CODE_COMBINING_MARKS_FOR_SYMBOLS = 0;
  const int BLOCK_CODE_LETTERLIKE_SYMBOLS = 0;
  const int BLOCK_CODE_NUMBER_FORMS = 0;
  const int BLOCK_CODE_ARROWS = 0;
  const int BLOCK_CODE_MATHEMATICAL_OPERATORS = 0;
  const int BLOCK_CODE_MISCELLANEOUS_TECHNICAL = 0;
  const int BLOCK_CODE_CONTROL_PICTURES = 0;
  const int BLOCK_CODE_OPTICAL_CHARACTER_RECOGNITION = 0;
  const int BLOCK_CODE_ENCLOSED_ALPHANUMERICS = 0;
  const int BLOCK_CODE_BOX_DRAWING = 0;
  const int BLOCK_CODE_BLOCK_ELEMENTS = 0;
  const int BLOCK_CODE_GEOMETRIC_SHAPES = 0;
  const int BLOCK_CODE_MISCELLANEOUS_SYMBOLS = 0;
  const int BLOCK_CODE_DINGBATS = 0;
  const int BLOCK_CODE_BRAILLE_PATTERNS = 0;
  const int BLOCK_CODE_CJK_RADICALS_SUPPLEMENT = 0;
  const int BLOCK_CODE_KANGXI_RADICALS = 0;
  const int BLOCK_CODE_IDEOGRAPHIC_DESCRIPTION_CHARACTERS = 0;
  const int BLOCK_CODE_CJK_SYMBOLS_AND_PUNCTUATION = 0;
  const int BLOCK_CODE_HIRAGANA = 0;
  const int BLOCK_CODE_KATAKANA = 0;
  const int BLOCK_CODE_BOPOMOFO = 0;
  const int BLOCK_CODE_HANGUL_COMPATIBILITY_JAMO = 0;
  const int BLOCK_CODE_KANBUN = 0;
  const int BLOCK_CODE_BOPOMOFO_EXTENDED = 0;
  const int BLOCK_CODE_ENCLOSED_CJK_LETTERS_AND_MONTHS = 0;
  const int BLOCK_CODE_CJK_COMPATIBILITY = 0;
  const int BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A = 0;
  const int BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS = 0;
  const int BLOCK_CODE_YI_SYLLABLES = 0;
  const int BLOCK_CODE_YI_RADICALS = 0;
  const int BLOCK_CODE_HANGUL_SYLLABLES = 0;
  const int BLOCK_CODE_HIGH_SURROGATES = 0;
  const int BLOCK_CODE_HIGH_PRIVATE_USE_SURROGATES = 0;
  const int BLOCK_CODE_LOW_SURROGATES = 0;
  const int BLOCK_CODE_PRIVATE_USE_AREA = 0;
  const int BLOCK_CODE_PRIVATE_USE = 0;
  const int BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS = 0;
  const int BLOCK_CODE_ALPHABETIC_PRESENTATION_FORMS = 0;
  const int BLOCK_CODE_ARABIC_PRESENTATION_FORMS_A = 0;
  const int BLOCK_CODE_COMBINING_HALF_MARKS = 0;
  const int BLOCK_CODE_CJK_COMPATIBILITY_FORMS = 0;
  const int BLOCK_CODE_SMALL_FORM_VARIANTS = 0;
  const int BLOCK_CODE_ARABIC_PRESENTATION_FORMS_B = 0;
  const int BLOCK_CODE_SPECIALS = 0;
  const int BLOCK_CODE_HALFWIDTH_AND_FULLWIDTH_FORMS = 0;
  const int BLOCK_CODE_OLD_ITALIC = 0;
  const int BLOCK_CODE_GOTHIC = 0;
  const int BLOCK_CODE_DESERET = 0;
  const int BLOCK_CODE_BYZANTINE_MUSICAL_SYMBOLS = 0;
  const int BLOCK_CODE_MUSICAL_SYMBOLS = 0;
  const int BLOCK_CODE_MATHEMATICAL_ALPHANUMERIC_SYMBOLS = 0;
  const int BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B = 0;
  const int BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT = 0;
  const int BLOCK_CODE_TAGS = 0;
  const int BLOCK_CODE_CYRILLIC_SUPPLEMENT = 0;
  const int BLOCK_CODE_CYRILLIC_SUPPLEMENTARY = 0;
  const int BLOCK_CODE_TAGALOG = 0;
  const int BLOCK_CODE_HANUNOO = 0;
  const int BLOCK_CODE_BUHID = 0;
  const int BLOCK_CODE_TAGBANWA = 0;
  const int BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A = 0;
  const int BLOCK_CODE_SUPPLEMENTAL_ARROWS_A = 0;
  const int BLOCK_CODE_SUPPLEMENTAL_ARROWS_B = 0;
  const int BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B = 0;
  const int BLOCK_CODE_SUPPLEMENTAL_MATHEMATICAL_OPERATORS = 0;
  const int BLOCK_CODE_KATAKANA_PHONETIC_EXTENSIONS = 0;
  const int BLOCK_CODE_VARIATION_SELECTORS = 0;
  const int BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_A = 0;
  const int BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_B = 0;
  const int BLOCK_CODE_LIMBU = 0;
  const int BLOCK_CODE_TAI_LE = 0;
  const int BLOCK_CODE_KHMER_SYMBOLS = 0;
  const int BLOCK_CODE_PHONETIC_EXTENSIONS = 0;
  const int BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_ARROWS = 0;
  const int BLOCK_CODE_YIJING_HEXAGRAM_SYMBOLS = 0;
  const int BLOCK_CODE_LINEAR_B_SYLLABARY = 0;
  const int BLOCK_CODE_LINEAR_B_IDEOGRAMS = 0;
  const int BLOCK_CODE_AEGEAN_NUMBERS = 0;
  const int BLOCK_CODE_UGARITIC = 0;
  const int BLOCK_CODE_SHAVIAN = 0;
  const int BLOCK_CODE_OSMANYA = 0;
  const int BLOCK_CODE_CYPRIOT_SYLLABARY = 0;
  const int BLOCK_CODE_TAI_XUAN_JING_SYMBOLS = 0;
  const int BLOCK_CODE_VARIATION_SELECTORS_SUPPLEMENT = 0;
  const int BLOCK_CODE_ANCIENT_GREEK_MUSICAL_NOTATION = 0;
  const int BLOCK_CODE_ANCIENT_GREEK_NUMBERS = 0;
  const int BLOCK_CODE_ARABIC_SUPPLEMENT = 0;
  const int BLOCK_CODE_BUGINESE = 0;
  const int BLOCK_CODE_CJK_STROKES = 0;
  const int BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT = 0;
  const int BLOCK_CODE_COPTIC = 0;
  const int BLOCK_CODE_ETHIOPIC_EXTENDED = 0;
  const int BLOCK_CODE_ETHIOPIC_SUPPLEMENT = 0;
  const int BLOCK_CODE_GEORGIAN_SUPPLEMENT = 0;
  const int BLOCK_CODE_GLAGOLITIC = 0;
  const int BLOCK_CODE_KHAROSHTHI = 0;
  const int BLOCK_CODE_MODIFIER_TONE_LETTERS = 0;
  const int BLOCK_CODE_NEW_TAI_LUE = 0;
  const int BLOCK_CODE_OLD_PERSIAN = 0;
  const int BLOCK_CODE_PHONETIC_EXTENSIONS_SUPPLEMENT = 0;
  const int BLOCK_CODE_SUPPLEMENTAL_PUNCTUATION = 0;
  const int BLOCK_CODE_SYLOTI_NAGRI = 0;
  const int BLOCK_CODE_TIFINAGH = 0;
  const int BLOCK_CODE_VERTICAL_FORMS = 0;
  const int BLOCK_CODE_NKO = 0;
  const int BLOCK_CODE_BALINESE = 0;
  const int BLOCK_CODE_LATIN_EXTENDED_C = 0;
  const int BLOCK_CODE_LATIN_EXTENDED_D = 0;
  const int BLOCK_CODE_PHAGS_PA = 0;
  const int BLOCK_CODE_PHOENICIAN = 0;
  const int BLOCK_CODE_CUNEIFORM = 0;
  const int BLOCK_CODE_CUNEIFORM_NUMBERS_AND_PUNCTUATION = 0;
  const int BLOCK_CODE_COUNTING_ROD_NUMERALS = 0;
  const int BLOCK_CODE_SUNDANESE = 0;
  const int BLOCK_CODE_LEPCHA = 0;
  const int BLOCK_CODE_OL_CHIKI = 0;
  const int BLOCK_CODE_CYRILLIC_EXTENDED_A = 0;
  const int BLOCK_CODE_VAI = 0;
  const int BLOCK_CODE_CYRILLIC_EXTENDED_B = 0;
  const int BLOCK_CODE_SAURASHTRA = 0;
  const int BLOCK_CODE_KAYAH_LI = 0;
  const int BLOCK_CODE_REJANG = 0;
  const int BLOCK_CODE_CHAM = 0;
  const int BLOCK_CODE_ANCIENT_SYMBOLS = 0;
  const int BLOCK_CODE_PHAISTOS_DISC = 0;
  const int BLOCK_CODE_LYCIAN = 0;
  const int BLOCK_CODE_CARIAN = 0;
  const int BLOCK_CODE_LYDIAN = 0;
  const int BLOCK_CODE_MAHJONG_TILES = 0;
  const int BLOCK_CODE_DOMINO_TILES = 0;
  const int BLOCK_CODE_SAMARITAN = 0;
  const int BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED = 0;
  const int BLOCK_CODE_TAI_THAM = 0;
  const int BLOCK_CODE_VEDIC_EXTENSIONS = 0;
  const int BLOCK_CODE_LISU = 0;
  const int BLOCK_CODE_BAMUM = 0;
  const int BLOCK_CODE_COMMON_INDIC_NUMBER_FORMS = 0;
  const int BLOCK_CODE_DEVANAGARI_EXTENDED = 0;
  const int BLOCK_CODE_HANGUL_JAMO_EXTENDED_A = 0;
  const int BLOCK_CODE_JAVANESE = 0;
  const int BLOCK_CODE_MYANMAR_EXTENDED_A = 0;
  const int BLOCK_CODE_TAI_VIET = 0;
  const int BLOCK_CODE_MEETEI_MAYEK = 0;
  const int BLOCK_CODE_HANGUL_JAMO_EXTENDED_B = 0;
  const int BLOCK_CODE_IMPERIAL_ARAMAIC = 0;
  const int BLOCK_CODE_OLD_SOUTH_ARABIAN = 0;
  const int BLOCK_CODE_AVESTAN = 0;
  const int BLOCK_CODE_INSCRIPTIONAL_PARTHIAN = 0;
  const int BLOCK_CODE_INSCRIPTIONAL_PAHLAVI = 0;
  const int BLOCK_CODE_OLD_TURKIC = 0;
  const int BLOCK_CODE_RUMI_NUMERAL_SYMBOLS = 0;
  const int BLOCK_CODE_KAITHI = 0;
  const int BLOCK_CODE_EGYPTIAN_HIEROGLYPHS = 0;
  const int BLOCK_CODE_ENCLOSED_ALPHANUMERIC_SUPPLEMENT = 0;
  const int BLOCK_CODE_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT = 0;
  const int BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C = 0;
  const int BLOCK_CODE_MANDAIC = 0;
  const int BLOCK_CODE_BATAK = 0;
  const int BLOCK_CODE_ETHIOPIC_EXTENDED_A = 0;
  const int BLOCK_CODE_BRAHMI = 0;
  const int BLOCK_CODE_BAMUM_SUPPLEMENT = 0;
  const int BLOCK_CODE_KANA_SUPPLEMENT = 0;
  const int BLOCK_CODE_PLAYING_CARDS = 0;
  const int BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS = 0;
  const int BLOCK_CODE_EMOTICONS = 0;
  const int BLOCK_CODE_TRANSPORT_AND_MAP_SYMBOLS = 0;
  const int BLOCK_CODE_ALCHEMICAL_SYMBOLS = 0;
  const int BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D = 0;
  const int BLOCK_CODE_ARABIC_EXTENDED_A = 0;
  const int BLOCK_CODE_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS = 0;
  const int BLOCK_CODE_CHAKMA = 0;
  const int BLOCK_CODE_MEETEI_MAYEK_EXTENSIONS = 0;
  const int BLOCK_CODE_MEROITIC_CURSIVE = 0;
  const int BLOCK_CODE_MEROITIC_HIEROGLYPHS = 0;
  const int BLOCK_CODE_MIAO = 0;
  const int BLOCK_CODE_SHARADA = 0;
  const int BLOCK_CODE_SORA_SOMPENG = 0;
  const int BLOCK_CODE_SUNDANESE_SUPPLEMENT = 0;
  const int BLOCK_CODE_TAKRI = 0;
  const int BLOCK_CODE_BASSA_VAH = 0;
  const int BLOCK_CODE_CAUCASIAN_ALBANIAN = 0;
  const int BLOCK_CODE_COPTIC_EPACT_NUMBERS = 0;
  const int BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_EXTENDED = 0;
  const int BLOCK_CODE_DUPLOYAN = 0;
  const int BLOCK_CODE_ELBASAN = 0;
  const int BLOCK_CODE_GEOMETRIC_SHAPES_EXTENDED = 0;
  const int BLOCK_CODE_GRANTHA = 0;
  const int BLOCK_CODE_KHOJKI = 0;
  const int BLOCK_CODE_KHUDAWADI = 0;
  const int BLOCK_CODE_LATIN_EXTENDED_E = 0;
  const int BLOCK_CODE_LINEAR_A = 0;
  const int BLOCK_CODE_MAHAJANI = 0;
  const int BLOCK_CODE_MANICHAEAN = 0;
  const int BLOCK_CODE_MENDE_KIKAKUI = 0;
  const int BLOCK_CODE_MODI = 0;
  const int BLOCK_CODE_MRO = 0;
  const int BLOCK_CODE_MYANMAR_EXTENDED_B = 0;
  const int BLOCK_CODE_NABATAEAN = 0;
  const int BLOCK_CODE_OLD_NORTH_ARABIAN = 0;
  const int BLOCK_CODE_OLD_PERMIC = 0;
  const int BLOCK_CODE_ORNAMENTAL_DINGBATS = 0;
  const int BLOCK_CODE_PAHAWH_HMONG = 0;
  const int BLOCK_CODE_PALMYRENE = 0;
  const int BLOCK_CODE_PAU_CIN_HAU = 0;
  const int BLOCK_CODE_PSALTER_PAHLAVI = 0;
  const int BLOCK_CODE_SHORTHAND_FORMAT_CONTROLS = 0;
  const int BLOCK_CODE_SIDDHAM = 0;
  const int BLOCK_CODE_SINHALA_ARCHAIC_NUMBERS = 0;
  const int BLOCK_CODE_SUPPLEMENTAL_ARROWS_C = 0;
  const int BLOCK_CODE_TIRHUTA = 0;
  const int BLOCK_CODE_WARANG_CITI = 0;
  const int BLOCK_CODE_COUNT = 0;
  const int BLOCK_CODE_INVALID_CODE = 0;

  const int EA_NEUTRAL = 0;
  const int EA_AMBIGUOUS = 0;
  const int EA_HALFWIDTH = 0;
  const int EA_FULLWIDTH = 0;
  const int EA_NARROW = 0;
  const int EA_WIDE = 0;
  const int EA_COUNT = 0;

  const int UNICODE_CHAR_NAME = 0;
  const int UNICODE_10_CHAR_NAME = 0;
  const int EXTENDED_CHAR_NAME = 0;

  const int CHAR_NAME_CHOICE_COUNT = 0;

  const int SHORT_PROPERTY_NAME = 0;
  const int LONG_PROPERTY_NAME = 0;
  const int PROPERTY_NAME_CHOICE_COUNT = 0;

  const int DT_NONE = 0;
  const int DT_CANONICAL = 0;
  const int DT_COMPAT = 0;
  const int DT_CIRCLE = 0;
  const int DT_FINAL = 0;
  const int DT_FONT = 0;
  const int DT_FRACTION = 0;
  const int DT_INITIAL = 0;
  const int DT_ISOLATED = 0;
  const int DT_MEDIAL = 0;
  const int DT_NARROW = 0;
  const int DT_NOBREAK = 0;
  const int DT_SMALL = 0;
  const int DT_SQUARE = 0;
  const int DT_SUB = 0;
  const int DT_SUPER = 0;
  const int DT_VERTICAL = 0;
  const int DT_WIDE = 0;
  const int DT_COUNT = 0;


  const int JT_NON_JOINING = 0;
  const int JT_JOIN_CAUSING = 0;
  const int JT_DUAL_JOINING = 0;
  const int JT_LEFT_JOINING = 0;
  const int JT_RIGHT_JOINING = 0;
  const int JT_TRANSPARENT = 0;
  const int JT_COUNT = 0;


  const int JG_NO_JOINING_GROUP = 0;
  const int JG_AIN = 0;
  const int JG_ALAPH = 0;
  const int JG_ALEF = 0;
  const int JG_BEH = 0;
  const int JG_BETH = 0;
  const int JG_DAL = 0;
  const int JG_DALATH_RISH = 0;
  const int JG_E = 0;
  const int JG_FEH = 0;
  const int JG_FINAL_SEMKATH = 0;
  const int JG_GAF = 0;
  const int JG_GAMAL = 0;
  const int JG_HAH = 0;



  const int JG_HAMZA_ON_HEH_GOAL = 0;
  const int JG_HE = 0;
  const int JG_HEH = 0;
  const int JG_HEH_GOAL = 0;
  const int JG_HETH = 0;
  const int JG_KAF = 0;
  const int JG_KAPH = 0;
  const int JG_KNOTTED_HEH = 0;
  const int JG_LAM = 0;
  const int JG_LAMADH = 0;
  const int JG_MEEM = 0;
  const int JG_MIM = 0;
  const int JG_NOON = 0;
  const int JG_NUN = 0;
  const int JG_PE = 0;
  const int JG_QAF = 0;
  const int JG_QAPH = 0;
  const int JG_REH = 0;
  const int JG_REVERSED_PE = 0;
  const int JG_SAD = 0;
  const int JG_SADHE = 0;
  const int JG_SEEN = 0;
  const int JG_SEMKATH = 0;
  const int JG_SHIN = 0;
  const int JG_SWASH_KAF = 0;
  const int JG_SYRIAC_WAW = 0;
  const int JG_TAH = 0;
  const int JG_TAW = 0;
  const int JG_TEH_MARBUTA = 0;
  const int JG_TETH = 0;
  const int JG_WAW = 0;
  const int JG_YEH = 0;
  const int JG_YEH_BARREE = 0;
  const int JG_YEH_WITH_TAIL = 0;
  const int JG_YUDH = 0;
  const int JG_YUDH_HE = 0;
  const int JG_ZAIN = 0;
  const int JG_FE = 0;
  const int JG_KHAPH = 0;
  const int JG_ZHAIN = 0;
  const int JG_BURUSHASKI_YEH_BARREE = 0;
  const int JG_COUNT = 0;

  const int GCB_OTHER = 0;
  const int GCB_CONTROL = 0;
  const int GCB_CR = 0;
  const int GCB_EXTEND = 0;
  const int GCB_L = 0;
  const int GCB_LF = 0;
  const int GCB_LV = 0;
  const int GCB_LVT = 0;
  const int GCB_T = 0;
  const int GCB_V = 0;
  const int GCB_SPACING_MARK = 0;
  const int GCB_PREPEND = 0;
  const int GCB_COUNT = 0;

  const int WB_OTHER = 0;
  const int WB_ALETTER = 0;
  const int WB_FORMAT = 0;
  const int WB_KATAKANA = 0;
  const int WB_MIDLETTER = 0;
  const int WB_MIDNUM = 0;
  const int WB_NUMERIC = 0;
  const int WB_EXTENDNUMLET = 0;
  const int WB_CR = 0;
  const int WB_EXTEND = 0;
  const int WB_LF = 0;
  const int WB_MIDNUMLET = 0;
  const int WB_NEWLINE = 0;
  const int WB_COUNT = 0;

  const int SB_OTHER = 0;
  const int SB_ATERM = 0;
  const int SB_CLOSE = 0;
  const int SB_FORMAT = 0;
  const int SB_LOWER = 0;
  const int SB_NUMERIC = 0;
  const int SB_OLETTER = 0;
  const int SB_SEP = 0;
  const int SB_SP = 0;
  const int SB_STERM = 0;
  const int SB_UPPER = 0;
  const int SB_CR = 0;
  const int SB_EXTEND = 0;
  const int SB_LF = 0;
  const int SB_SCONTINUE = 0;
  const int SB_COUNT = 0;

  const int LB_UNKNOWN = 0;
  const int LB_AMBIGUOUS = 0;
  const int LB_ALPHABETIC = 0;
  const int LB_BREAK_BOTH = 0;
  const int LB_BREAK_AFTER = 0;
  const int LB_BREAK_BEFORE = 0;
  const int LB_MANDATORY_BREAK = 0;
  const int LB_CONTINGENT_BREAK = 0;
  const int LB_CLOSE_PUNCTUATION = 0;
  const int LB_COMBINING_MARK = 0;
  const int LB_CARRIAGE_RETURN = 0;
  const int LB_EXCLAMATION = 0;
  const int LB_GLUE = 0;
  const int LB_HYPHEN = 0;
  const int LB_IDEOGRAPHIC = 0;
  const int LB_INSEPARABLE = 0;
  const int LB_INSEPERABLE = 0;
  const int LB_INFIX_NUMERIC = 0;
  const int LB_LINE_FEED = 0;
  const int LB_NONSTARTER = 0;
  const int LB_NUMERIC = 0;
  const int LB_OPEN_PUNCTUATION = 0;
  const int LB_POSTFIX_NUMERIC = 0;
  const int LB_PREFIX_NUMERIC = 0;
  const int LB_QUOTATION = 0;
  const int LB_COMPLEX_CONTEXT = 0;
  const int LB_SURROGATE = 0;
  const int LB_SPACE = 0;
  const int LB_BREAK_SYMBOLS = 0;
  const int LB_ZWSPACE = 0;
  const int LB_NEXT_LINE = 0;
  const int LB_WORD_JOINER = 0;
  const int LB_H2 = 0;
  const int LB_H3 = 0;
  const int LB_JL = 0;
  const int LB_JT = 0;
  const int LB_JV = 0;
  const int LB_COUNT = 0;

  const int NT_NONE = 0;
  const int NT_DECIMAL = 0;
  const int NT_DIGIT = 0;
  const int NT_NUMERIC = 0;
  const int NT_COUNT = 0;

  const int HST_NOT_APPLICABLE = 0;
  const int HST_LEADING_JAMO = 0;
  const int HST_VOWEL_JAMO = 0;
  const int HST_TRAILING_JAMO = 0;
  const int HST_LV_SYLLABLE = 0;
  const int HST_LVT_SYLLABLE = 0;
  const int HST_COUNT = 0;

  static public function chr($cp) { }
  static public function ord($cp) { }
  static public function hasBinaryProperty($cp, int $prop) { }
  static public function getIntPropertyValue($cp, int $prop) { }
  static public function getIntPropertyMaxValue(int $prop): int { return 0; }
  static public function getIntPropertyMinValue(int $prop): int { return 0; }
  static public function getNumericValue($cp) { }
  static public function enumCharTypes((function(int,int,int):void) $cb): void { }
  static public function getBlockCode($cp) { }
  static public function charName($cp, int $choice = IntlChar::UNICODE_CHAR_NAME) { }
  static public function charFromName(string $name, int $choice = IntlChar::UNICODE_CHAR_NAME) { }
  static public function enumCharNames($start, $limit, (function(int,int,string):void) $cb, int $choice = IntlChar::UNICODE_CHAR_NAME): void { }
  static public function getPropertyName(int $prop, int $choice = IntlChar::LONG_PROPERTY_NAME) { }
  static public function getPropertyValueName(int $prop, int $value, int $choice = IntlChar::LONG_PROPERTY_NAME) { }
  static public function getPropertyEnum(string $alias): int { return 0; }
  static public function getPropertyValueEnum(int $prop, string $name): int { return 0; }
  static public function foldCase($cp, int $options = IntlChar::FOLD_CASE_DEFAULT) { }
  static public function digit($cp, int $radix = 10) { }
  static public function forDigit(int $digit, int $radix = 10): int { return 0; }
  static public function charAge($cp) { }
  static public function getUnicodeVersion(): array { return array(); }
  static public function getFC_NFKC_Closure($cp) { }
  static public function isUAlphabetic($cp) { }
  static public function isULowercase($cp) { }
  static public function isUUppercase($cp) { }
  static public function isUWhiteSpace($cp) { }
  static public function islower($cp) { }
  static public function isupper($cp) { }
  static public function istitle($cp) { }
  static public function isdigit($cp) { }
  static public function isalpha($cp) { }
  static public function isalnum($cp) { }
  static public function isxdigit($cp) { }
  static public function ispunct($cp) { }
  static public function isgraph($cp) { }
  static public function isblank($cp) { }
  static public function isdefined($cp) { }
  static public function isspace($cp) { }
  static public function isJavaSpaceChar($cp) { }
  static public function isWhitespace($cp) { }
  static public function iscntrl($cp) { }
  static public function isISOControl($cp) { }
  static public function isprint($cp) { }
  static public function isbase($cp) { }
  static public function isMirrored($cp) { }
  static public function isIDStart($cp) { }
  static public function isIDPart($cp) { }
  static public function isIDIgnorable($cp) { }
  static public function isJavaIDStart($cp) { }
  static public function isJavaIDPart($cp) { }
  static public function charDirection($cp) { }
  static public function charType($cp) { }
  static public function getCombiningClass($cp) { }
  static public function charDigitValue($cp) { }
  static public function getBidiPairedBracket($cp) { }
  static public function charMirror($cp) { }
  static public function tolower($cp) { }
  static public function toupper($cp) { }
  static public function totitle($cp) { }
}
