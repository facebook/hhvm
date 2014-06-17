<?hh

interface I1 {}

if (time() > 0) {
  interface I2 {
    require implements I1;
  }
}

echo 'Fail';
