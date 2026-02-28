<?hh


function catches(): void {
  try {
  } catch (Exception $_) {
  }
  try {
  } catch (Throwable $_) {
  }
  try {
  } catch (Error $_) {
  }
}

function throws(): void {
  throw new Exception();
  throw new Error();
  throw new TypeError();
}
