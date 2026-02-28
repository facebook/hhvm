<?hh

enum MyExitCode: int {
  SUCCESS = 0;
  FAILURE = 1;
}

function main_function_early_terminate(): int {
  if (1 === 1) {
    return 0;
  }

  return 0;
}

function main_function_early_terminate_enum(): int {
  if (1 === 1) {
    return MyExitCode::SUCCESS;
  }

  return MyExitCode::SUCCESS;
}
