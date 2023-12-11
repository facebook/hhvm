<?hh

function VS($x, $y) :mixed{
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) :mixed{ VS($x != false, true); }


//////////////////////////////////////////////////////////////////////

<<__EntryPoint>>
function main_ext_mailparse() :mixed{
$text =
  "To: fred@bloggs.com\n".
  "Content-Type: multipart/mixed;\n".
  "\tboundary=\"----=_NextPart_\"\n".
  "\n".
  "This is a multi-part message in MIME format.\n".
  "\n".
  "------=_NextPart_\n".
  "Content-Type: tex/plain;\n".
  "\tcharset=\"us-ascii\"\n".
  "Content-Transfer-Encoding: 7bit\n".
  "\n".
  "this is a regular mime attachment.\n".
  "\n";

// MAXPARTS is 300, but the error does not occur until the 302nd
// MIME part is parsed.
for ($i = 0; $i < 301; $i++) {
  $text .=
    "------=_NextPart_\n".
    "Content-Type: application/octet-stream;\n".
    "\tname=\"README{$i}\"\n".
    "Content-Transfer-Encoding: 7bit\n".
    "Content-Disposition: attachment;;\n".
    "\tfilename=\"README{$i}\"\n".
    "\n".
    "Part{$i}\n".
    "\n";
}

$text .= "------=_NextPart_--\n";

$mime = mailparse_msg_create();
$result = mailparse_msg_parse($mime, $text);
VS($result, false);

VS(ezmlm_hash("foo"), 40);

$files = vec["mime", "phpcvs1", "qp", "uue"];

foreach ($files as $file) {
  $testname = __DIR__."/test_ext_mailparse." . $file . ".txt";

  $mime = mailparse_msg_create();
  $input = fopen($testname, "r");
  while (!feof($input)) {
    $data = fread($input, 1024);
    if ($data) {
      mailparse_msg_parse($mime, $data);
    }
  }

  $arr = mailparse_msg_get_structure($mime);
  echo "Message: ";
  echo $file;
  echo "\n";

  foreach ($arr as $blahblah => $partname) {
    $depth = count(explode(".", $partname)) - 1;
    $indent = str_repeat("  ", $depth * 2);

    $subpart = mailparse_msg_get_part($mime, $partname);
    if (!$subpart) {
      var_dump($partname); echo("\n");
      var_dump($arr);
      break;
    }

    $data = mailparse_msg_get_part_data($subpart);
    echo "\n"; echo $indent; echo "Part "; echo $partname; echo "\n";
    ksort(inout $data);
    foreach ($data as $key => $second) {
      if ($key != "headers" && $key != "ending-pos-body") {
        echo $indent; echo $key; echo " => ";
        var_dump($second);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////

$text =
  "To: fred@bloggs.com\n".
  "To: wez@thebrainroom.com\n".
  "\n".
  "hello, this is some text=hello.";

$mime = mailparse_msg_create();
mailparse_msg_parse($mime, $text);
$data = mailparse_msg_get_part_data($mime);
VS($data['headers']['to'], "fred@bloggs.com, wez@thebrainroom.com");

//////////////////////////////////////////////////////////////////////

$text =
  "To: fred@bloggs.com\n".
  "Mime-Version: 1.0\n".
  "Content-Type: text/plain\n".
  "Subject: A simple MIME message\n".
  "\n".
  "hello, this is some text hello.\n".
  "blah blah blah.\n";

$fp = tmpfile();
fwrite($fp, $text);
rewind($fp);

ob_start();

$mime = mailparse_msg_create();
mailparse_msg_parse($mime, $text);

echo("Extract to output\n");
mailparse_msg_extract_part_file($mime, $fp);

echo("Extract and return as string\n");
$result = mailparse_msg_extract_part_file($mime, $fp, null);
echo("-->\n");
echo($result);

echo("\nExtract to open file\n");
$fpdest = tmpfile();
mailparse_msg_extract_part_file($mime, $fp, $fpdest);
echo("\nrewinding\n");
rewind($fpdest);
fpassthru($fpdest);

echo("\nExtract via user function\n");
mailparse_msg_extract_part_file($mime, $fp);

echo("\nExtract whole part to output\n");
mailparse_msg_extract_whole_part_file($mime, $fp);

echo("\nExtract part from string to output\n");
mailparse_msg_extract_part($mime, $text);
fclose($fpdest);
fclose($fp);

$output = ob_get_contents();
ob_end_clean();

VS($output,
   "Extract to output\n".
   "hello, this is some text hello.\n".
   "blah blah blah.\n".
   "Extract and return as string\n".
   "-->\n".
   "hello, this is some text hello.\n".
   "blah blah blah.\n".
   "\n".
   "Extract to open file\n".
   "\n".
   "rewinding\n".
   "hello, this is some text hello.\n".
   "blah blah blah.\n".
   "\n".
   "Extract via user function\n".
   "hello, this is some text hello.\n".
   "blah blah blah.\n".
   "\n".
   "Extract whole part to output\n".
   "To: fred@bloggs.com\n".
   "Mime-Version: 1.0\n".
   "Content-Type: text/plain\n".
   "Subject: A simple MIME message\n".
   "\n".
   "hello, this is some text hello.\n".
   "blah blah blah.\n".
   "\n".
   "Extract part from string to output\n".
   "hello, this is some text hello.\n".
   "blah blah blah.\n");

//////////////////////////////////////////////////////////////////////

$msg =
  "Received: from mail pickup service by hotmail.com with Microsoft\n".
  "SMTPSVC;\n".
  "Sat, 18 Feb 2006 22:58:14 -0800\n".
  "Received: from 66.178.40.49 by BAY116-DAV8.phx.gbl with DAV;\n".
  "Sun, 19 Feb 2006 06:58:13 +0000\n".
  "\n".
  "test";

$mail = mailparse_msg_create();
mailparse_msg_parse($mail, $msg);
$arr = mailparse_msg_get_structure($mail);
foreach ($arr as $first => $second) {
  $section = mailparse_msg_get_part($mail, $second);
  $info = mailparse_msg_get_part_data($section);
  $received =
    vec["from mail pickup service by hotmail.com with Microsoft",
                   "from 66.178.40.49 by BAY116-DAV8.phx.gbl with DAV;"];
  VS($info,
    dict[
           "headers" => dict["received" => $received],
           "starting-pos" => 0,
           "starting-pos-body" => 200,
           "ending-pos" => 200,
           "ending-pos-body" => 200,
           "line-count" => 6,
           "body-line-count" => 0,
           "charset" => "us-ascii",
           "transfer-encoding" => "8bit",
           "content-type" => "text/plain",
           "content-base" => "/"
    ]
  );
}

//////////////////////////////////////////////////////////////////////

$addresses =
  vec["\":sysmail\"@ Some-Group. Some-Org, Muhammed.".
                 "(I am the greatest) Ali @(the)Vegas.WBA",
                 "\"strange\":\":sysmail\"@ Some-Group. Some-Org, Muhammed.".
                 "(I am the greatest) Ali @(the)Vegas.WBA;"];

ob_start();

foreach ($addresses as $first => $second) {
  $parsed = mailparse_rfc822_parse_addresses($second);
  foreach ($parsed as $pfirst => $psecond) {
    $pair = $psecond;
    echo($pair['display']); echo("\n");
    echo($pair['address']); echo("\n");
    if ($pair['is_group']) {
      $sub = mailparse_rfc822_parse_addresses
        (substr($pair['address'], 1, strlen($pair['address']) - 2));
      foreach ($sub as $blah) {
        echo("   "); echo($blah['address']); echo("\n");
      }
    }
  }
  echo("...\n");
}

$output = ob_get_contents();
ob_end_clean();
VS($output,
   ":sysmail@Some-Group.Some-Org\n".
   "\":sysmail\"@Some-Group.Some-Org\n".
   "I am the greatest the\n".
   "Muhammed.Ali@Vegas.WBA\n".
   "...\n".
   "strange\n".
   ":\":sysmail\"@Some-Group.Some-Org,Muhammed.Ali@Vegas.WBA;\n".
   "   \":sysmail\"@Some-Group.Some-Org\n".
   "   Muhammed.Ali@Vegas.WBA\n".
   "...\n");

//////////////////////////////////////////////////////////////////////

  $text = "hello, this is some text=hello.";
  $fp = tmpfile();
  fwrite($fp, $text); rewind($fp);
  $dest = tmpfile();
  mailparse_stream_encode($fp, $dest, "quoted-printable");
  rewind($dest);
  $data = fread($dest, 2048);
  VS($data, "hello, this is some text=3Dhello.");

  $text =
    "To: fred@bloggs.com\n".
    "\n".
    "blah blah blah From blah $ \" & £ blah blah blah blah blah\n".
    "From the first of the month, things will be different!\n".
    "blah blah blah From blah\n".
    "Frome is a town in Somerset.";

  ob_start();

  $fp = tmpfile();
  fwrite($fp, $text);
  rewind($fp);

  $fpdest = tmpfile();
  mailparse_stream_encode($fp, $fpdest, "quoted-printable");
  rewind($fpdest);
  fpassthru($fpdest);

  fclose($fp);
  fclose($fpdest);

  $output = ob_get_contents();
  ob_end_clean();
  VS($output,
     "To: fred@bloggs.com\r\n".
     "\r\n".
     "blah blah blah From blah $ \" & =C2=A3 blah blah blah blah blah\r\n".
     "=46rom the first of the month, things will be different!\r\n".
     "blah blah blah From blah\r\n".
     "Frome is a town in Somerset.");

//////////////////////////////////////////////////////////////////////

$text =
  "To: fred@bloggs.com\n".
  "\n".
  "hello, this is some text hello.\n".
  "blah blah blah.\n".
  "\n".
  "begin 644 test.txt\n".
  "/=&AI<R!I<R!A('1E<W0*\n".
  "`\n".
  "end";

ob_start();

$fp = tmpfile();
fwrite($fp, $text);

$data = mailparse_uudecode_all($fp);
echo("BODY\n");
readfile($data[0]['filename']);
echo("UUE\n");
readfile($data[1]['filename']);

unlink($data[0]['filename']);
unlink($data[1]['filename']);

$output = ob_get_contents();
ob_end_clean();
VS($output,
   "BODY\n".
   "To: fred@bloggs.com\n".
   "\n".
   "hello, this is some text hello.\n".
   "blah blah blah.\n".
   "\n".
   "UUE\n".
   "this is a test\n");
}
