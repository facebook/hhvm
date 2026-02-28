<?hh

interface I {}
interface J {}

function takes_string(string $_): void {}

function main(I $i, J $j): void {
  if ($i === $j) {
    // Points to the equality as reason for type of $i
    takes_string($i);
  }
}
