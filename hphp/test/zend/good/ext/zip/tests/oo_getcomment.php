<?hh
$dirname = dirname(__FILE__) . '/';
$file = $dirname . 'test_with_comment.zip';
include $dirname . 'utils.inc';
$zip = new ZipArchive;
if (!$zip->open($file)) {
	exit('failed');
}
echo $zip->getArchiveComment() . "\n";

$idx = $zip->locateName('foo');
echo $zip->getCommentName('foo') . "\n";
echo $zip->getCommentIndex($idx);

echo $zip->getCommentName('') . "\n";
try { echo $zip->getCommentName() . "\n"; } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

$zip->close();

