# pyre-unsafe
import os

import common_tests


class TestRenameSoundDynamic(common_tests.CommonTests):
    @classmethod
    def get_test_driver(cls) -> common_tests.CommonTestDriver:
        return common_tests.CommonTestDriver()

    def write_load_config(self):
        with open(os.path.join(self.test_driver.repo_dir, ".hhconfig"), "w") as f:
            f.write(
                """
enable_sound_dynamic_type = true
"""
            )

    def write_and_test_one_file(
        self, file_input, command_type, element_name, upcast_locations, using_sd=True
    ) -> None:
        with open(os.path.join(self.test_driver.repo_dir, "a.php"), "w") as f:
            f.write(file_input)
        if using_sd:
            self.write_load_config()
        self.test_driver.start_hh_server(changed_files=["a.php"], args=["--no-load"])
        self.check_upcast_cmd(command_type, element_name, upcast_locations, using_sd)

    def check_upcast_cmd(
        self, command_type, element_name, upcast_locations, using_sd=True
    ):
        if using_sd:
            expected_output = [
                "Server is using sound dynamic. ",
                f"Number of upcast positions for \\{element_name} is {len(upcast_locations)}",
            ]
            expected_output.extend(upcast_locations)
        else:
            expected_output = [
                "Server is NOT using sound dynamic. Change the .hhconfig file to enable sound dynamic. ",
            ]

        self.test_driver.check_cmd(
            expected_output=expected_output,
            options=["--refactor-sound-dynamic", command_type, element_name],
        )

    def test_no_sd(self) -> None:
        self.write_and_test_one_file(
            """<?hh
            <<file:__EnableUnstableFeatures('upcast_expression')>>
            <<__SupportDynamicType>>
            function h(shape(...) $m): int {
              $x = h<> upcast dynamic;
            }
            """,
            "Function",
            "h",
            [],
            using_sd=False,
        )

    def test_one_upcast_one_function(self) -> None:

        self.write_and_test_one_file(
            """<?hh
            <<file:__EnableUnstableFeatures('upcast_expression')>>
            <<__SupportDynamicType>>
            function h(shape(...) $m): int {
              $x = h<> upcast dynamic;
            }
            """,
            "Function",
            "h",
            [
                f'File "{self.test_driver.repo_dir}/a.php", line 5, characters 20-37:',
            ],
        )

    def test_one_upcast_one_class(self) -> None:

        self.write_and_test_one_file(
            """
<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>
<<__SupportDynamicType>>
class Counter {
  private int $i = 0;
}

function c(dynamic $d): void {
  $g = Counter::class upcast dynamic;
  $h = new Counter() upcast dynamic;
}

            """,
            "Class",
            "Counter",
            [
                f'File "{self.test_driver.repo_dir}/a.php", line 10, characters 8-36:',
                f'File "{self.test_driver.repo_dir}/a.php", line 11, characters 8-35:',
            ],
        )

    def test_one_upcast_multiple_function(self) -> None:
        self.write_and_test_one_file(
            """<?hh
            <<file:__EnableUnstableFeatures('upcast_expression')>>
            <<__SupportDynamicType>>
            function h(shape(...) $m): int {
              $x = h<> upcast dynamic;
            }
            <<__SupportDynamicType>>
            function g(int $m): int {
              $x = g<> upcast dynamic;
            }
            """,
            "Function",
            "h",
            [
                f'File "{self.test_driver.repo_dir}/a.php", line 5, characters 20-37:',
            ],
        )

    def test_multiple_files(self) -> None:
        with open(os.path.join(self.test_driver.repo_dir, "a.php"), "w") as f:
            f.write(
                """<?hh
            <<file:__EnableUnstableFeatures('upcast_expression')>>
            <<__SupportDynamicType>>
            function h(shape(...) $m): int {
              $x = h<> upcast dynamic;
            }
            <<__SupportDynamicType>>
            function g(int $m): int {
              $x = g<> upcast dynamic;
            }
            """
            )

        with open(os.path.join(self.test_driver.repo_dir, "b.php"), "w") as f:
            f.write(
                """<?hh
                <<file:__EnableUnstableFeatures('upcast_expression')>>
                <<__SupportDynamicType>>
                function b(dynamic $d): void {
                  $y = h<> upcast dynamic;
                  $d(3);
                }
                """
            )

        with open(os.path.join(self.test_driver.repo_dir, "c.php"), "w") as f:
            f.write(
                """<?hh
                <<file:__EnableUnstableFeatures('upcast_expression')>>
                <<__SupportDynamicType>>
                function c(int $i): void {
                  $z = h<> upcast dynamic;
                }
                """
            )

        self.write_load_config()
        self.test_driver.start_hh_server(
            changed_files=["a.php", "b.php", "c.php"], args=["--no-load"]
        )
        self.check_upcast_cmd(
            "Function",
            "h",
            [
                f'File "{self.test_driver.repo_dir}/a.php", line 5, characters 20-37:',
                f'File "{self.test_driver.repo_dir}/b.php", line 5, characters 24-41:',
                f'File "{self.test_driver.repo_dir}/c.php", line 5, characters 24-41:',
            ],
        )
