<?hh

function duplicate_with_default(): void {
  switch (true) {
    case true:
      return;
    case true:
      return;
    default:
      return;
  }
}

function duplicate_on_unrecognised(): void {
  switch (vec[]) {
    case 1:
      return;
    case 1:
      return;
    default:
      return;
  }
}

function duplicate_distant(): void {
  switch (true) {
    case true:
      return;
    case false:
      return;
    case true:
      return;
  }
}
