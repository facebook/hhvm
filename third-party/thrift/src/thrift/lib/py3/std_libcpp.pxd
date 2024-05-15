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

# distutils: language = c++

# stdlib functions that have not been wrapped in the cython distribution
from libc.stdint cimport int64_t
from libcpp cimport bool as cbool

cdef extern from "<chrono>" namespace "std::chrono" nogil:
    cdef cppclass milliseconds:
        milliseconds(int64_t) except +
        int64_t count()

    cdef cppclass seconds:
        seconds(int64_t) except +

cdef extern from "<iterator>" namespace "std" nogil:
    cdef cppclass iterator_traits[T]:
        cppclass difference_type:
            pass
    cdef iterator_traits[InputIter].difference_type distance[InputIter](
        InputIter first,
        InputIter second)

    cdef InputIter next[InputIter](
        InputIter it,
        int64_t n)

    cdef InputIter prev[InputIter](
        InputIter it,
        int64_t n)

cdef extern from "<algorithm>" namespace "std" nogil:
    cdef InputIter find[InputIter, T](
        InputIter first,
        InputIter second,
        const T& val)
    cdef iterator_traits[InputIter].difference_type count[InputIter, T](
        InputIter first,
        InputIter second,
        const T& val)
    cdef cbool includes[Iter1, Iter2](
        Iter1 first1,
        Iter1 last1,
        Iter2 first2,
        Iter2 last2)
    cdef OutputIter set_intersection[Iter1, Iter2, OutputIter](
        Iter1 first1,
        Iter1 last1,
        Iter2 first2,
        Iter2 last2,
        OutputIter result)
    cdef OutputIter set_difference[Iter1, Iter2, OutputIter](
        Iter1 first1,
        Iter1 last1,
        Iter2 first2,
        Iter2 last2,
        OutputIter result)
    cdef OutputIter set_union[Iter1, Iter2, OutputIter](
        Iter1 first1,
        Iter1 last1,
        Iter2 first2,
        Iter2 last2,
        OutputIter result)
    cdef OutputIter set_symmetric_difference[Iter1, Iter2, OutputIter](
        Iter1 first1,
        Iter1 last1,
        Iter2 first2,
        Iter2 last2,
        OutputIter result)

cdef extern from "<string_view>" namespace "std" nogil:
    cdef cppclass string_view "std::string_view":
        string_view()
        string_view(const char*)
        size_t size()
        bint empty()
        const char* data()

cdef inline str sv_to_str(string_view sv) except +:
    return sv.data().decode("utf-8")
