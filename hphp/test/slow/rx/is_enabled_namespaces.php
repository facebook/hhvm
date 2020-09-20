<?hh

// The \HH\Rx\IS_ENABLED constant exists at compile time only, but is
// auto-imported from the HH namespace magically, just like it would be at
// runtime. Test this here.

namespace Unrelated {
  <<__Rx>>
  function test_explicit() {
    if (\HH\Rx\IS_ENABLED) {
      return true;
    } else {
      echo "FAIL\n";
      return false;
    }
  }

  <<__Rx>>
  function test_autoimported() {
    if (Rx\IS_ENABLED) {
      return true;
    } else {
      echo "FAIL\n";
      return false;
    }
  }
}

namespace HH {
  <<__Rx>>
  function test_explicit() {
    if (\HH\Rx\IS_ENABLED) {
      return true;
    } else {
      echo "FAIL\n";
      return false;
    }
  }

  <<__Rx>>
  function test_implicit() {
    if (Rx\IS_ENABLED) {
      return true;
    } else {
      echo "FAIL\n";
      return false;
    }
  }
}

namespace HH\Rx {
  <<__Rx>>
  function test_explicit() {
    if (\HH\Rx\IS_ENABLED) {
      return true;
    } else {
      echo "FAIL\n";
      return false;
    }
  }

  <<__Rx>>
  function test_implicit() {
    if (IS_ENABLED) {
      return true;
    } else {
      echo "FAIL\n";
      return false;
    }
  }

  <<__Rx>>
  function test_autoimported() {
    if (Rx\IS_ENABLED) {
      return true;
    } else {
      echo "FAIL\n";
      return false;
    }
  }
}

namespace {
  <<__Rx>>
  function test_explicit() {
    if (\HH\Rx\IS_ENABLED) {
      return true;
    } else {
      echo "FAIL\n";
      return false;
    }
  }

  <<__Rx>>
  function test_autoimported() {
    if (Rx\IS_ENABLED) {
      return true;
    } else {
      echo "FAIL\n";
      return false;
    }
  }

  <<__EntryPoint>>
  function main() {
    \var_dump(\Unrelated\test_explicit());
    \var_dump(\Unrelated\test_autoimported());

    \var_dump(\HH\test_explicit());
    \var_dump(\HH\test_implicit());

    \var_dump(\HH\Rx\test_explicit());
    \var_dump(\HH\Rx\test_implicit());
    \var_dump(\HH\Rx\test_autoimported());

    \var_dump(\test_explicit());
    \var_dump(\test_autoimported());
  }
}
