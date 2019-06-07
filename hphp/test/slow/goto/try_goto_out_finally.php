<?hh
  try {
  } finally {
    try {
       goto foo;
    }
    finally {}
  }
  foo:
