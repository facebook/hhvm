<?hh

/* conformance to Unicode 5.2, section 3.9, D92 */

abstract final class ZendGoodExtStandardTestsStringsHtmlentitiesUtf3 {
  public static $val_ranges;
}

function is_valid($seq) :mixed{

	$b = ord($seq[0]);
	foreach (ZendGoodExtStandardTestsStringsHtmlentitiesUtf3::$val_ranges as $l) {
		if ($b >= $l[0][0] && $b <= $l[0][1]) {
			if (count($l) != strlen($seq)) {
				return false;
			}
			for ($n = 1; $n < strlen($seq); $n++) {
				if (ord($seq[$n]) < $l[$n][0] || ord($seq[$n]) > $l[$n][1]) {
					return false;
				}
			}
			return true;
		}
	}
	return false;
}

function concordance($s) :mixed{
	$vhe = strlen(htmlspecialchars($s, ENT_QUOTES, "UTF-8")) > 0;
	$v = is_valid($s);
	return ($vhe === $v);
}
<<__EntryPoint>>
function entrypoint_htmlentitiesutf3(): void {

  ZendGoodExtStandardTestsStringsHtmlentitiesUtf3::$val_ranges = vec[
  	vec[vec[0x00, 0x7F]],
  	vec[vec[0xC2, 0xDF], vec[0x80, 0xBF]],
  	vec[vec[0xE0, 0xE0], vec[0xA0, 0xBF], vec[0x80, 0xBF]],
  	vec[vec[0xE1, 0xEC], vec[0x80, 0xBF], vec[0x80, 0xBF]],
  	vec[vec[0xED, 0xED], vec[0x80, 0x9F], vec[0x80, 0xBF]],
  	vec[vec[0xEE, 0xEF], vec[0x80, 0xBF], vec[0x80, 0xBF]],
  	vec[vec[0xF0, 0xF0], vec[0x90, 0xBF], vec[0x80, 0xBF], vec[0x80, 0xBF]],
  	vec[vec[0xF1, 0xF3], vec[0x80, 0xBF], vec[0x80, 0xBF], vec[0x80, 0xBF]],
  	vec[vec[0xF4, 0xF4], vec[0x80, 0x8F], vec[0x80, 0xBF], vec[0x80, 0xBF]],
  ];

  for ($b1 = 0xC0; $b1 < 0xE0; $b1++) {
  	for ($b2 = 0x80; $b2 < 0xBF; $b2++) {
  		$s = chr($b1).chr($b2);
  		if (!concordance($s))
  			echo "Discordance for ".bin2hex($s),"\n";
  	}
  }


  for ($b1 = 0xE0; $b1 < 0xEF; $b1++) {
  	for ($b2 = 0x80; $b2 < 0xBF; $b2++) {
  		$s = chr($b1).chr($b2)."\x80";
  		if (!concordance($s))
  			echo "Discordance for ".bin2hex($s),"\n";
  		$s = chr($b1).chr($b2)."\xBF";
  		if (!concordance($s))
  			echo "Discordance for ".bin2hex($s),"\n";
  	}
  }

  for ($b1 = 0xF0; $b1 < 0xFF; $b1++) {
  	for ($b2 = 0x80; $b2 < 0xBF; $b2++) {
  		$s = chr($b1).chr($b2)."\x80\x80";
  		if (!concordance($s))
  			echo "Discordance for ".bin2hex($s),"\n";
  		$s = chr($b1).chr($b2)."\xBF\x80";
  		if (!concordance($s))
  			echo "Discordance for ".bin2hex($s),"\n";
  		$s = chr($b1).chr($b2)."\x80\xBF";
  		if (!concordance($s))
  			echo "Discordance for ".bin2hex($s),"\n";
  		$s = chr($b1).chr($b2)."\xBF\xBF";
  		if (!concordance($s))
  			echo "Discordance for ".bin2hex($s),"\n";
  	}
  }
  echo "Done.\n";
}
