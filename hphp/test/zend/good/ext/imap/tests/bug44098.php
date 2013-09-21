<?php
$exp = 'Luzon®14 dot CoM';
$res = imap_utf8('=?iso-8859-1?b?THV6b26uMTQ=?= dot CoM');
if ($res != $exp) {
	echo "failed: got <$res>, expected <exp>\n";
} else {
	echo "ok";
}
?>