<?hh // strict

const darray<string, int> DATA = dict["foo" => 42];

function main(): void {
    DATA["bar"] = -1;
}
