<?hh
<<file:__EnableUnstableFeatures('case_types')>>

enum StrOpaque : string {}
enum IntOpaque : int {}

enum StrEnum : string as string {}
enum IntEnum : int as int {}

case type StrData =
  | StrOpaque
  | IntOpaque
  | StrEnum
  | string;

case type IntData =
  | StrOpaque
  | IntOpaque
  | IntEnum
  | int;
