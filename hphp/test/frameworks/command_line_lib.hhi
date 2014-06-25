<?hh // decl

// See hphp/tools/command_line_lib.php

function error(string $message): void;
type OptionInfo    = Pair<string,string>;
type OptionInfoMap = Map<string,OptionInfo>;
type OptionMap     = Map<string,mixed>;

function parse_options(OptionInfoMap $optmap): OptionMap;
function display_help(string $message, OptionInfoMap $optmap): void;
