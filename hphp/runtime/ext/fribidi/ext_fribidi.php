<?hh

<<__Native>>
function fribidi_log2vis(
  string $logical_str,
  int $direction,
  int $charset,
): mixed;

<<__Native>>
function fribidi_charset_info(int $charset): shape(
  "name" => string,
  "title" => string,
  ?"desc" => string,
);

<<__Native>>
function fribidi_get_charsets(): darray<string, shape(
  "name" => string,
  "title" => string,
  ?"desc" => string,
)>;
