<?hh
function my_error_handler($errno, $errmsg, $filename, $linenum, $vars)
:mixed{
	echo "$errno: $errmsg\n";
}

function do_single_test($header)
:mixed{


	$result = iconv_mime_decode($header, ZendGoodExtIconvTestsIconvMimeDecode::$mode, "UTF-8");
	if ($result === false) {
		printf("(%d) \"%s\"\n", 0, '');
	} else {
		printf("(%d) \"%s\"\n", iconv_strlen($result, "UTF-8"), $result);
	}
}

function do_regression_test()
:mixed{
	do_single_test(<<< HERE
Subject: =?ISO-8859-1?Q?Pr=FCfung?=
	=?ISO-8859-1*de_DE?Q?Pr=FCfung?=\t
 	 =?ISO-8859-2?Q?k=F9=D4=F1=D3let?=
HERE
);
	do_single_test(<<< HERE
Subject: =?ISO-8859-1?Q?Pr=FCfung?= =?ISO-8859-1*de_DE?Q?=20Pr=FCfung?= \t  =?ISO-8859-2?Q?k=F9=D4=F1=D3let?=
HERE
);
	do_single_test(<<< HERE
Subject: =?ISO-8859-1?Q?Pr=FCfung?==?ISO-8859-1*de_DE?Q?Pr=FCfung?==?ISO-8859-2?Q?k=F9=D4=F1=D3let?=
HERE
);
	do_single_test(<<< HERE
Subject: =?ISO-8859-1?Q?Pr=FCfung?= =?ISO-8859-1*de_DE?Q?Pr=FCfung??   =?ISO-8859-2?X?k=F9=D4=F1=D3let?=
HERE
);
	do_single_test(<<< HERE
From: =?ISO-2022-JP?B?GyRCJTUbKEI=?=
 =?ISO-2022-JP?B?GyRCJXMlVxsoQg==?=
 =?ISO-2022-JP?B?GyRCJWtKOBsoQg==?=
 =?ISO-2022-JP?B?GyRCO3pOcxsoQg==?=
 =?ISO-2022-JP?B?GyRCJTUlcxsoQg==?=
 =?ISO-2022-JP?B?GyRCJVclaxsoQg==?=
 =?ISO-2022-JP?B?GyRCSjg7ehsoQg==?=
 =?ISO-2022-JP?B?GyRCTnNGfBsoQg==?=
 =?ISO-2022-JP?B?GyRCS1w4bBsoQg==?=
 =?ISO-2022-JP?B?GyRCJUYlLRsoQg==?=
 =?ISO-2022-JP?B?GyRCJTklSBsoQg==?=
HERE
);
}

abstract final class ZendGoodExtIconvTestsIconvMimeDecode {
  public static $mode;
}
<<__EntryPoint>>
function entrypoint_iconv_mime_decode(): void {
  set_error_handler(my_error_handler<>);

  ZendGoodExtIconvTestsIconvMimeDecode::$mode = 0;
  do_regression_test();
  ZendGoodExtIconvTestsIconvMimeDecode::$mode = ICONV_MIME_DECODE_STRICT;
  do_regression_test();
  ZendGoodExtIconvTestsIconvMimeDecode::$mode = ICONV_MIME_DECODE_CONTINUE_ON_ERROR;
  do_regression_test();
  ZendGoodExtIconvTestsIconvMimeDecode::$mode = ICONV_MIME_DECODE_STRICT | ICONV_MIME_DECODE_CONTINUE_ON_ERROR;
  do_regression_test();
}
