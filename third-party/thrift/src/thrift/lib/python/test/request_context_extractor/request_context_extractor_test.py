#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import unittest

from thrift.python.test.request_context_extractor.request_context_extractor import (
    PyRequestContextExtractor,
)


class RequestContextExtractorTest(unittest.TestCase):
    """
    Test suite for PyRequestContextExtractor to verify error handling
    when different incorrect types are supplied to the extraction methods.
    """

    def test_extract_cpp_request_context_with_none_input(self):
        self.assertRaisesRegex(
            TypeError,
            r"^Cannot convert.*RequestContext",
            PyRequestContextExtractor.extract_cpp_request_context,
            None,
        )

        self.assertRaisesRegex(
            TypeError,
            r"^Cannot convert.*ConnectionContext",
            PyRequestContextExtractor.extract_cpp_connection_context,
            None,
        )

    def test_extract_cpp_request_context_with_string_input(self):
        self.assertRaisesRegex(
            TypeError,
            r"^Cannot convert.*RequestContext",
            PyRequestContextExtractor.extract_cpp_request_context,
            "not_a_context",
        )

        self.assertRaisesRegex(
            TypeError,
            r"^Cannot convert.*ConnectionContext",
            PyRequestContextExtractor.extract_cpp_connection_context,
            "not_a_context",
        )

    def test_extract_cpp_request_context_with_integer_input(self):
        self.assertRaisesRegex(
            TypeError,
            r"^Cannot convert.*RequestContext",
            PyRequestContextExtractor.extract_cpp_request_context,
            42,
        )

        self.assertRaisesRegex(
            TypeError,
            r"^Cannot convert.*ConnectionContext",
            PyRequestContextExtractor.extract_cpp_connection_context,
            42,
        )

    def test_edge_case_types(self):
        edge_cases = [
            lambda: None,  # function
            type,  # type object
            Exception(),  # exception object
            self,  # test object itself
            bytes(b"test"),  # bytes
            bytearray(b"test"),  # bytearray
        ]

        for edge_case in edge_cases:
            with self.subTest(edge_case_type=type(edge_case).__name__):
                self.assertRaisesRegex(
                    TypeError,
                    r"^Cannot convert.*RequestContext",
                    PyRequestContextExtractor.extract_cpp_request_context,
                    edge_case,
                )

                self.assertRaisesRegex(
                    TypeError,
                    r"^Cannot convert.*ConnectionContext",
                    PyRequestContextExtractor.extract_cpp_connection_context,
                    edge_case,
                )


if __name__ == "__main__":
    unittest.main()
