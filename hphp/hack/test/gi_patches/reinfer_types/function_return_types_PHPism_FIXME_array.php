//// file1.php
<?hh

/* HH_FIXME[2071] Fixme'd to not have thousands of FIXMEs in WWW (for now) */
 type PHPism_FIXME_Array = varray_or_darray;

//// file2.php
<?hh // partial

function fa(): PHPism_FIXME_Array {
  return varray[4];
}

//// file3.php
<?hh // partial

function fb(): PHPism_FIXME_Array {
  return darray[4 => "why type?"];
}
