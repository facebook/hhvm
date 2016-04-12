#.rst:
# FindIntl
# --------
#
# Find the Gettext libintl headers and libraries.
#
# This module reports information about the Gettext libintl
# installation in several variables.  General variables::
#
#   Intl_FOUND - true if the libintl headers and libraries were found
#   Intl_INCLUDE_DIRS - the directory containing the libintl headers
#   Intl_LIBRARIES - libintl libraries to be linked
#
# The following cache variables may also be set::
#
#   Intl_INCLUDE_DIR - the directory containing the libintl headers
#   Intl_LIBRARY - the libintl library (if any)
#
# .. note::
#   On some platforms, such as Linux with GNU libc, the gettext
#   functions are present in the C standard library and libintl
#   is not required.  ``Intl_LIBRARIES`` will be empty in this
#   case.
#
# .. note::
#   If you wish to use the Gettext tools (``msgmerge``,
#   ``msgfmt``, etc.), use :module:`FindGettext`.


# Written by Roger Leigh <rleigh@codelibre.net>

#=============================================================================
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2015 Kitware, Inc.
# Copyright 2000-2011 Insight Software Consortium
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the names of Kitware, Inc., the Insight Software Consortium,
#   nor the names of their contributors may be used to endorse or promote
#   products derived from this software without specific prior written
#   permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ------------------------------------------------------------------------------
#
# The above copyright and license notice applies to distributions of
# CMake in source and binary form.  Some source files contain additional
# notices of original copyright by their contributors; see each source
# for details.  Third-party software packages supplied with CMake under
# compatible licenses provide their own copyright notices documented in
# corresponding subdirectories.
#
# ------------------------------------------------------------------------------
#
# CMake was initially developed by Kitware with the following sponsorship:
#
#  * National Library of Medicine at the National Institutes of Health
#    as part of the Insight Segmentation and Registration Toolkit (ITK).
#
#  * US National Labs (Los Alamos, Livermore, Sandia) ASC Parallel
#    Visualization Initiative.
#
#  * National Alliance for Medical Image Computing (NAMIC) is funded by the
#    National Institutes of Health through the NIH Roadmap for Medical Research,
#    Grant U54 EB005149.
#
#  * Kitware, Inc.
#
#=============================================================================

# Find include directory
find_path(LIBINTL_INCLUDE_DIR
          NAMES "libintl.h"
          DOC "libintl include directory")
mark_as_advanced(LIBINTL_INCLUDE_DIR)

# Find all Intl libraries
find_library(LIBINTL_LIBRARY "intl"
  DOC "libintl libraries (if not in the C library)")
mark_as_advanced(LIBINTL_LIBRARY)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBINTL
                                  REQUIRED_VARS LIBINTL_INCLUDE_DIR
                                  FAIL_MESSAGE "Failed to find Gettext libintl")

if(LIBINTL_FOUND)
  set(LIBINTL_INCLUDE_DIRS "${LIBINTL_INCLUDE_DIR}")
  if(LIBINTL_LIBRARY)
    set(LIBINTL_LIBRARIES "${LIBINTL_LIBRARY}")
  else()
    unset(LIBINTL_LIBRARIES)
  endif()
endif()
