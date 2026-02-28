<?hh

enum IntEnum: int as int {
    X = 1;
}

enum StringEnum: string as string {
    X = '1';
}

function takes_int_enum(IntEnum $_): void {}

function takes_string_enum(StringEnum $_): void {}

<<__EntryPoint>>
function main() :mixed{
    $val = '1';
    $val is IntEnum ? IntEnum::assert($val) |> takes_int_enum($$) : null;
    $val is StringEnum ? StringEnum::assert($val) |> takes_string_enum($$) : null;
}
