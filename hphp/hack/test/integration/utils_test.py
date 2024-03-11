# pyre-unsafe
import unittest

from utils import interpolate_variables, uninterpolate_variables


class UtilsTest(unittest.TestCase):
    def test_interpolate_variables_simple(self) -> None:
        variables = {"foo": "bar"}
        payload = {"hello": "hi ${foo} hi"}
        expected = {"hello": "hi bar hi"}
        self.assertEqual(interpolate_variables(payload, variables), expected)
        self.assertEqual(uninterpolate_variables(expected, variables), payload)

    def test_interpolate_variables_multiple(self) -> None:
        variables = {"foo": "bar", "baz": "qux"}
        payload = {
            "hello": "${foo} ${baz}",
            "nested": {"foo": "${foo}"},
            "in key: ${baz}": True,
        }
        expected = {"hello": "bar qux", "nested": {"foo": "bar"}, "in key: qux": True}
        self.assertEqual(interpolate_variables(payload, variables), expected)
        self.assertEqual(uninterpolate_variables(expected, variables), payload)

    def test_undefined_variable_raises_exception(self) -> None:
        variables = {}
        payload = {"hello": "bar ${foo} bar"}
        with self.assertRaises(ValueError):
            interpolate_variables(payload, variables)
