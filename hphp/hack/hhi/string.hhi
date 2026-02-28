<?hh

namespace HH {

  <<__SupportDynamicType>>
  final class string {
    // All methods are ignored unless ignore_string_methods is set to false in .hhconfig
    <<__ImplementedBy('\HH\String\__primitive_length')>>
    public function length(): int;
  }

  namespace String {
    // All functions are ignored unless ignore_string_methods is set to false in .hhconfig
    <<__SupportDynamicType>>
    function __primitive_length(string $s): int;
  }
}
