#include <fstream>
#include <cstdio>

#include <folly/portability/GTest.h>

#include "hphp/util/process.h"

namespace HPHP {

class ProcPressureTest : public ::testing::TestWithParam<std::pair<std::string, ProcPressure>> {
protected:
    char tmp_filename[L_tmpnam];
    FILE* tmpFile;

    void SetUp() override {
      if (tmpnam(tmp_filename) == NULL) {
        FAIL() << "File could not be created.";
      }

      tmpFile = fopen(tmp_filename, "w");
    }

    void TearDown() override {
      int result = fclose(tmpFile);
      if (result != 0) {
        FAIL() << "Failed to close temporary file" << std::endl;
      }
    }
};

void checkAssertions(const ProcPressure& pressure, const ProcPressure& expected) {
  ASSERT_EQ(pressure.some.has_value(), expected.some.has_value());
  if (expected.some) {
    EXPECT_EQ(pressure.some->avg10, expected.some->avg10);
    EXPECT_EQ(pressure.some->avg60, expected.some->avg60);
    EXPECT_EQ(pressure.some->avg300, expected.some->avg300);
    EXPECT_EQ(pressure.some->total, expected.some->total);
  }

  ASSERT_EQ(pressure.full.has_value(), expected.full.has_value());
  if (expected.full) {
    EXPECT_EQ(pressure.full->avg10, expected.full->avg10);
    EXPECT_EQ(pressure.full->avg60, expected.full->avg60);
    EXPECT_EQ(pressure.full->avg300, expected.full->avg300);
    EXPECT_EQ(pressure.full->total, expected.full->total);
  }
}

TEST_P(ProcPressureTest, TestPressure) {
  auto [input, expected] = GetParam();

  size_t n = fwrite(input.c_str(), sizeof(char), strlen(input.c_str()), tmpFile);
  if (n != strlen(input.c_str())) {
    FAIL() << "Failed to write data to temporary file" << std::endl;
  }
  fflush(tmpFile);

  ProcPressure cpuPressure = Process::GetCPUPressure(tmp_filename);
  checkAssertions(cpuPressure, expected);

  ProcPressure ioPressure = Process::GetIOPressure(tmp_filename);
  checkAssertions(ioPressure, expected);

  ProcPressure memoryPressure = Process::GetMemoryPressure(tmp_filename);
  checkAssertions(memoryPressure, expected);
}

INSTANTIATE_TEST_SUITE_P(
    Params,
    ProcPressureTest,
    ::testing::Values(
      // Both "some" and "full" are present, random values
      std::make_pair<std::string, ProcPressure>(
        "some avg10=1.11 avg60=2.22 avg300=3.33 total=1234\n"
        "full avg10=4.44 avg60=5.55 avg300=6.66 total=4567\n",
        ProcPressure{
          .some = Pressure{.avg10 = 1.11, .avg60 = 2.22, .avg300 = 3.33, .total = 1234},
          .full = Pressure{.avg10 = 4.44, .avg60 = 5.55, .avg300 = 6.66, .total = 4567}
        }
      ),
      // Only "some" is present
      std::make_pair<std::string, ProcPressure>(
        "some avg10=1.11 avg60=2.22 avg300=3.33 total=1234\n",
        ProcPressure{
          .some = Pressure{.avg10 = 1.11, .avg60 = 2.22, .avg300 = 3.33, .total = 1234},
          .full = std::nullopt
        }
      ),
      // Only "full" is present
      std::make_pair<std::string, ProcPressure>(
        "full avg10=4.44 avg60=5.55 avg300=6.66 total=4567\n",
        ProcPressure{
          .some = std::nullopt,
          .full = Pressure{.avg10 = 4.44, .avg60 = 5.55, .avg300 = 6.66, .total = 4567}
        }
      ),
      // File is empty
      std::make_pair<std::string, ProcPressure>(
        "",
        ProcPressure{
          .some = std::nullopt,
          .full = std::nullopt
        }
      ),
      // File is invalid
      std::make_pair<std::string, ProcPressure>(
        "<invalid>\n",
        ProcPressure{
          .some = std::nullopt,
          .full = std::nullopt
        }
      )
  )
);

} // namespace HPHP
