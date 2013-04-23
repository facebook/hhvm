<?php
var_dump(
  grapheme_substr('FOK', 1, 20), // expected: OK
  grapheme_substr('한국어', 1, 20) //expected: 국어
);