<?hh

namespace {
  class Integer {
    const int MAX32 = INT32_MAX;
    const int MAX64 = Math\INT64_MAX;
  }

  namespace NS {
    class Integer {
      const int MAX32 = INT32_MAX;
      const int MAX64 = Math\INT64_MAX;
    }
  }
}
