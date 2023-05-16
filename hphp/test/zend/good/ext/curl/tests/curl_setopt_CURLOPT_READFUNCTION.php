<?hh
function custom_readfunction($oCurl, $hReadHandle, $iMaxOut)
{
  $sData = fread($hReadHandle,$iMaxOut-10); // -10 to have space to add "custom:"
  if ($sData ?? false)
  {
    $sData = "custom:".$sData;
  }
  return $sData;
}

<<__EntryPoint>> function main(): void {
$sReadFile  = sys_get_temp_dir().'/'.'in.tmp';
$sWriteFile = sys_get_temp_dir().'/'.'out.tmp';
$sWriteUrl  = 'file://'.$sWriteFile;

file_put_contents($sReadFile,'contents of tempfile');
$hReadHandle = fopen($sReadFile, 'r');

$oCurl = curl_init();
curl_setopt($oCurl, CURLOPT_URL,          $sWriteUrl);
curl_setopt($oCurl, CURLOPT_UPLOAD,       1);
curl_setopt($oCurl, CURLOPT_READFUNCTION, custom_readfunction<> );
curl_setopt($oCurl, CURLOPT_INFILE,       $hReadHandle );
curl_exec($oCurl);
curl_close($oCurl);

fclose ($hReadHandle);

$sOutput = file_get_contents($sWriteFile);
var_dump($sOutput);
echo "===DONE===\n";

unlink($sReadFile);
unlink($sWriteFile);
}
