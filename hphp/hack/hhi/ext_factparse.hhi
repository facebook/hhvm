<?hh

namespace HH {

function facts_parse(
  ?string $root,
  varray<string> $pathList,
  bool $allowHipHopSyntax,
  bool $useThreads,
): darray<string, ?darray<string, mixed>>;

function ext_factparse_version(): int;

} // namespace HH
