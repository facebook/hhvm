<?php

$adds = 'ian eiloart <iane@example.ac.uk>,
      shuf6@example.ac.uk,
      blobby,
      "ian,eiloart"<ian@example.ac.uk>,
      <@example.com:foo@example.ac.uk>,
      foo@#,
      ian@-example.com,
      ian@one@two';
$add_arr = imap_rfc822_parse_adrlist($adds, 'example.com');
var_export($add_arr);

?>