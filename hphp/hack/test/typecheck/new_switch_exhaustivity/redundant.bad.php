<?hh

function redundant_case(): void {
  switch (true) {
    case true:
      return;
    case false:
      return;
    case 1:
      return;
  }
}

function redundant_case_with_default(): void {
  switch (true) {
    case true:
      return;
    case false:
      return;
    case 1:
      return;
    default:
      return;
  }
}

function redundant_default(): void {
  switch (true) {
    case true:
      return;
    case false:
      return;
    default:
      return;
  }
}
