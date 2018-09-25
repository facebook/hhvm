<?hh // strict

const vec<int> DATA = vec[42];

function main(): void {
    DATA[1] = -1;
}
