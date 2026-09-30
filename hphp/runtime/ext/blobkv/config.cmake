HHVM_DEFINE_EXTENSION("blobkv"
  SOURCES
    ext_blobkv.cpp
  SYSTEMLIB
    ext_blobkv-reader.php
    ext_blobkv-writer.php
)
