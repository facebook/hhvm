<?hh //decl

namespace HH {

function facts_parse(
  ?string $root,
  array<string> $pathList,
  bool $allowHipHopSyntax,
  bool $useThreads,
): array<string, ?array<string, mixed>>;

function ext_factparse_version(): int;

} // namespace HH
