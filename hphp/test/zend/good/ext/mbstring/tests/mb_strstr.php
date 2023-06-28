<?hh
function EUC_JP($utf8str) :mixed{
	return mb_convert_encoding($utf8str, "EUC-JP", "UTF-8");
}

function FROM_EUC_JP($eucjpstr) :mixed{
	return mb_convert_encoding($eucjpstr, "UTF-8", "EUC-JP");
}
<<__EntryPoint>>
function main_entry(): void {

  var_dump(mb_strstr("あいうえおかきくけこ", "おかき"));
  var_dump(mb_strstr("あいうえおかきくけこ", "おかき", false));
  var_dump(mb_strstr("あいうえおかきくけこ", "おかき", true));
  var_dump(FROM_EUC_JP(mb_strstr(EUC_JP("あいうえおかきくけこ"), EUC_JP("おかき"), false, "EUC-JP")));
  var_dump(FROM_EUC_JP(mb_strstr(EUC_JP("あいうえおかきくけこ"), EUC_JP("おかき"), true, "EUC-JP")));
  mb_internal_encoding("EUC-JP");
  var_dump(FROM_EUC_JP(mb_strstr(EUC_JP("あいうえおかきくけこ"), EUC_JP("おかき"))));
  var_dump(FROM_EUC_JP(mb_strstr(EUC_JP("あいうえおかきくけこ"), EUC_JP("おかき"), false)));
  var_dump(FROM_EUC_JP(mb_strstr(EUC_JP("あいうえおかきくけこ"), EUC_JP("おかき"), true)));
}
