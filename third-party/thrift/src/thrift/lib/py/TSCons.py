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

from __future__ import absolute_import, division, print_function, unicode_literals

from os import path

from SCons.Builder import Builder


def scons_env(env, add=""):
    opath = path.dirname(path.abspath("$TARGET"))
    lstr = "thrift --gen cpp -o " + opath + " " + add + " $SOURCE"
    cppbuild = Builder(action=lstr)
    env.Append(BUILDERS={"ThriftCpp": cppbuild})


def gen_cpp(env, dir, file):
    scons_env(env)
    suffixes = ["_data.h", "_data.cpp", "_types.h", "_types.cpp"]
    targets = ["gen-cpp" + file + s for s in suffixes]
    return env.ThriftCpp(targets, dir + file + ".thrift")
