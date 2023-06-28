<?hh

// Copyright 2004-present Facebook. All Rights Reserved.

// Generate 10k random bytes
<<__EntryPoint>>
function main_zip_stream() :mixed{
srand(42);
$num_bytes = 10000;
$orig_bytes = '';
for ($i = 0; $i < $num_bytes; ++$i) {
  $orig_bytes = $orig_bytes . chr(rand(1, 127));
}

print 'len($orig_bytes) = '.strlen($orig_bytes)."\n";

$tf = tempnam(sys_get_temp_dir(), 'data.txt');
$tfh = fopen($tf, "w");
fwrite($tfh, $orig_bytes, $num_bytes);
fclose($tfh);

$tfz = tempnam(sys_get_temp_dir(), 'data.txt.zip');
$zf = new ZipArchive();
$zf->open($tfz);
try {
  $zf->addFile($tf, 'foobar');
} finally {
  $zf->close();
}

$zf = new ZipArchive();
$zf->open($tfz);
try {
  $fh = $zf->getStream('foobar');
  try {
    $zipread = stream_get_contents($fh, 9000);
    print 'stream_get_contents($fh, 9000) on zip stream = '.strlen($zipread)."\n";
  } finally {
    fclose($fh);
  }
  $fh = $zf->getStream('foobar');
  try {
    $zipreadfull = stream_get_contents($fh);
    print 'stream_get_contents($fh) on zip stream = '.strlen($zipreadfull)."\n";
  } finally {
    fclose($fh);
  }
} finally {
  $zf->close();
}
}
