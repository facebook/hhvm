# Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

INCLUDE(CheckCSourceRuns)
INCLUDE(CheckCXXSourceRuns)
INCLUDE(CMakePushCheckState)

SET(code "
  int main (int argc, char **argv)
  {
    double n[21] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 1, 1, 1, 1, 1, 1, 1,1 };
    double m= 0, s= 0;
    int i;
    for(i= 0; i < 21; i++)
    {
      double m_kminusone= m;
      m= m_kminusone + (n[i] - m_kminusone) / (double) (i + 2);
      s= s + (n[i] - m_kminusone) * (n[i] - m);
    }
    /*
      s should now be either 5e 74 d1 45 17 5d 14 40 or
      40 14 5d 17 45 d1 74 5e, depending on endianness. If the floating point
      operations are optimized with fused multiply-add instructions, the least
      significant byte is 5d instead of 5e.
    */
    return (*(unsigned char*)(&s) == 0x5e ||
            *((unsigned char*)(&s) + 7) == 0x5e);
  }"
)

CMAKE_PUSH_CHECK_STATE()
STRING_APPEND(CMAKE_REQUIRED_FLAGS " -O3")

IF(MY_COMPILER_IS_GNU)
  CHECK_C_SOURCE_RUNS("${code}" HAVE_C_FLOATING_POINT_FUSED_MADD)
  CHECK_CXX_SOURCE_RUNS("${code}" HAVE_CXX_FLOATING_POINT_FUSED_MADD)
ENDIF()

CMAKE_POP_CHECK_STATE()
