<?hh

const FOO = "string";
const BAR = FOO; // identifying FOO will not work because we
                 // don't typecheck global constants in IDE mode
