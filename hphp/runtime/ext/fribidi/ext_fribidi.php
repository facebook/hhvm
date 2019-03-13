<?hh // partial

<<__Native>>
function fribidi_log2vis(
  string $logical_str,
  int $direction,
  int $charset,
): mixed;

<<__Native>>
function fribidi_charset_info(int $charset): array;

<<__Native>>
function fribidi_get_charsets(): array;
