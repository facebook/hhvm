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


def create_ThriftUnicodeDecodeError_from_UnicodeDecodeError(error, field_name):
    if isinstance(error, ThriftUnicodeDecodeError):
        error.field_names.append(field_name)
        return error
    return ThriftUnicodeDecodeError(
        error.encoding, error.object, error.start, error.end, error.reason, field_name
    )


class ThriftUnicodeDecodeError(UnicodeDecodeError):
    def __init__(self, encoding, object, start, end, reason, field_name):
        super(ThriftUnicodeDecodeError, self).__init__(
            encoding, object, start, end, reason
        )
        self.field_names = [field_name]

    def __str__(self):
        return "{error} when decoding field '{field}'".format(
            error=super(ThriftUnicodeDecodeError, self).__str__(),
            field="->".join(reversed(self.field_names)),
        )
