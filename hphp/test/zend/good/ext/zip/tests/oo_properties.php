<?hh
<<__EntryPoint>>
function main(): void {
  $dirname = dirname(__FILE__).'/';
  $file = sys_get_temp_dir().'/'.'__property_test.zip';

  copy($dirname.'test_with_comment.zip', $file);

  $zip = new ZipArchive;
  if (!$zip->open($file)) {
    exit('failed');
  }

  printf(
    "zip->status (%d):\n\tisset(): %d\n",
    $zip->status,
    isset($zip->status),
  );
  printf(
    "zip->numFiles (%d):\n\tisset(): %d\n",
    $zip->numFiles,
    isset($zip->numFiles),
  );
  try {
    printf(
      "zip->bogus (%d):\n\tisset(): %d\n",
      $zip->bogus,
      isset($zip->bogus),
    );
  } catch (UndefinedPropertyException $e) {
    var_dump($e->getMessage());
  }

  $zip->addEmptyDir('emptydir');

  printf(
    "zip->status (%d):\n\tisset(): %d\n",
    $zip->status,
    isset($zip->status),
  );
  printf(
    "zip->numFiles (%d):\n\tisset(): %d\n",
    $zip->numFiles,
    isset($zip->numFiles),
  );
  printf(
    "zip->filename (%d):\n\tisset(): %d\n",
    strlen($zip->filename),
    isset($zip->filename),
  );
  printf(
    "zip->comment (%d):\n\tisset(): %d\n",
    strlen($zip->comment),
    isset($zip->comment),
  );

  unset($zip); //close the file before unlinking
  unlink($file);
}
