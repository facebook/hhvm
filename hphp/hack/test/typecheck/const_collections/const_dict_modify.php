<?hh // strict

const dict<string, int> DATA = dict["foo" => 42];

function main(): void {
    DATA["bar"] = -1;
}
