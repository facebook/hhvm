/* Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2.0,
as published by the Free Software Foundation.

This program is also distributed with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have included with MySQL.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License, version 2.0, for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @page GENERAL_DEVELOPMENT_GUIDELINES General Development Guidelines

  We use Git for source management.

  You should use the TRUNK source tree (currently called
  "mysql-trunk") for all new development. To download and set
  up the public development branch, use these commands:

  ~~~~~~~~~~~~~~~~
  shell> git clone https://github.com/mysql/mysql-server.git mysql-trunk
  shell> cd mysql-trunk
  shell> git branch mysql-trunk
  ~~~~~~~~~~~~~~~~
  Before making big design decisions, please begin by posting a
  summary of what you want to do, why you want to do it, and
  how you plan to do it. This way we can easily provide you
  with feedback and also discuss it thoroughly. Perhaps another
  developer can assist you.

  - @subpage CODING_GUIDELINES_OF_SERVER
  - @subpage NAMING_CONVENTION
  - @subpage COMMENTING_CODE
  - @subpage HEADER_FILE
  - @subpage EXAMPLE_SETUP_FOR_CTAGS
*/

/**
  @page CODING_GUIDELINES_OF_SERVER Legacy C++ Coding Guidelines of MySQL Server

  This section covers guidelines for C++ code for the MySQL
  server, old code only. (New code should follow Google C++ coding style.)
  The guidelines do not necessarily apply for other
  projects such as MySQL Connector/J or Connector/ODBC.
*/

/**
  @page NAMING_CONVENTION Legacy Naming Conventions

  - For identifiers formed from multiple words, separate each
    component with underscore rather than capitalization.
    Thus, use my_var instead of myVar or MyVar.

  - Avoid capitalization except for class names; class names
    should begin with a capital letter.
    This exception from Google coding guidelines exists
    because the server has a history of using My_class. It will
    be confusing to mix the two (from a code-review perspective).

  ~~~~~~~~~~~~~~~~
  class Item;
  class Query_arena;
  class Log_event;
  ~~~~~~~~~~~~~~~~

  - Avoid function names, structure elements, or variables
    that begin or end with '_'.

  - Use long function and variable names in English. This
    makes your code easier to read for all developers.

  - We used to have the rule: "Structure types are typedef'ed
    to an all-upper-case identifier." That rule has been
    deprecated for C++ code. Do not add typedefs for
    structs/classes in C++.

  - All \#define declarations should be in upper case.

  ~~~~~~~~~~~~~~~~
  #define MY_CONSTANT 15
  ~~~~~~~~~~~~~~~~

  - Enumeration names should begin with enum_.

  - Function declarations (forward declarations) have
    parameter names in addition to parameter types.
*/

/**
  @page COMMENTING_CODE Legacy Commenting Style

  - Comment your code when you do something that someone else
    may think is not trivial.

  - Comments for pure virtual functions, documentation for
    API usage should precede (member, or
    non-member) function declarations. Descriptions of
    implementation details, algorithms, anything that does
    not impact usage, should precede the
    implementation. Please try to not duplicate information.
    Make a reference to the declaration from the
    implementation if necessary. If the implementation and
    usage are too interleaved, put a reference from the
    interface to the implementation, and keep the entire
    comment in a single place.

  - Class comments should precede the class
    declaration.

  - When writing multi-line comments please put the '<em>*</em>' and
    '<em>*</em>/' on their own lines, put the '<em>*</em>/' below the
    '/<em>*</em>', put a line break and a two-space indent after the
    '/<em>*</em>', do not use additional asterisks on the left of the comment.

  <div style="margin-left:30px">
  <table style="background-color:#E0E0E0"><tr><td style="width:670px"><pre>
  /<em>*</em>
    This is how a multi-line comment in the middle of code
    should look.  Note it is not Doxygen-style if it is not at the
    beginning of a code enclosure (function or class).
  <em>*</em>/

  /<em>* *********</em>This comment is bad. It's indented incorrectly, it has
  <em>*</em>            additional asterisks. Don't write this way.
  <em>*  **********</em>/</pre>
  </td></tr></table></div>

  - When writing single-line comments, the '/<em>*</em>' and '<em>*</em>/" are
    on the same line. For example:

  <div style="margin-left:30px">
  <table style="background-color:#E0E0E0"><tr><td style="width:670px">
  /<em>*</em> We must check if stack_size = Solaris 2.9 can return 0 here.
  <em>*</em>/</td></tr></table></div>

  - Single-line comments like this are okay in C++.

  <div style="margin-left:30px">
  <table style="background-color:#E0E0E0"><tr><td style="width:670px">
  /<em></em>/ We must check if stack_size = Solaris 2.9 can return 0 here.
  </td></tr></table></div>


  - For a short comment at the end of a line, you may use
    either /<em>*</em> ... *<em></em>/ or a // double slash. In C files or in
    header files used by C files, avoid // comments.

  - Align short side // or /<em>*</em> ... <em>*</em>/ comments by 48th column
    (start the comment in column 49).

  <div style="margin-left:30px">
  <table style="background-color:#E0E0E0"><tr><td style="width:670px">
  { qc*= 2; /<em>*</em> double the estimation <em>*</em>/ }
  </td></tr></table></div>

  - All comments should be in English.

  - Each standalone comment must start with a Capital letter.

  - There is a '.' at the end of each statement in a comment
    paragraph, including the last one.

  <div style="margin-left:30px">
  <table style="background-color:#E0E0E0"><tr><td style="width:670px"><pre>
  /<em>*</em>
    This is a standalone comment. The comment is aligned to fit 79
    characters per line. There is a period at the end of each sentence.
    Including the last one.
  <em>*</em>/</pre>
  </td></tr></table></div>

 - Every structure, class, method or function should have a
   description unless it is very short and its purpose is
   obvious.

 - Use the following example as a template for function or
   method comments.

      + Please refer to the Doxygen Manual
        (http://www.stack.nl/~dimitri/doxygen/manual.html)
        for additional information.

      + Note the IN and OUT parameters. IN is implicit, and
        can (but usually should not) be specified with the
        \@param[in] tag. For OUT and INOUT parameters you should
        use \@param[out] and \@param[in,out] tags,
        respectively.

      + Parameter specifications in \@param section start
        with lowercase and are not terminated with a full
        stop/period.

      + Section headers are aligned at two spaces. This must
        be a sentence with a full stop/period at the end.
        If the sentence must express a subject that
        contains a full stop such that Doxygen would be
        fooled into stopping early, then use \@brief and
        \@details to explicitly mark them.

      + Align \@retval specifications at four spaces if they
        follow a \@return description. Else, align at two
        spaces.

      + Separate sections with an empty line. <br>

      + All function comments should be no longer than 79
        characters per line.

      + Put two line breaks (one empty line) between a
        function comment and its description. <br>

      + Doxygen comments: Use <em>/</em>** ... *<em>/</em> syntax and not ///

      + Doxygen command: Use '@' and not '\' for doxygen commands.

  <div style="margin-left:30px">
  <table style="background-color:#E0E0E0"><tr><td style="width:670px">
  /<em>**</em><pre>
    Initialize SHA1Context.

    Set initial values in preparation for computing a new SHA1 message digest.

    \@param[in,out]  context  the context to reset

    \@return Operation status
      \@retval SHA_SUCCESS      OK
      \@retval != SHA_SUCCESS   sha error Code
  <em>*</em>/

  int sha1_reset(SHA1_CONTEXT *context)
  {
    ...</pre>
  </td></tr></table></div>
*/

/**
  @page HEADER_FILE Header Files

  - Use header guards. Put the header guard in the first
    line of the header, before the copyright. Use an
    all-uppercase name for the header guard. Derive the
    header guard name from the file base name, and append
    _INCLUDED to create a macro name. Example: sql_show.h ->
    SQL_SHOW_INCLUDED.

  - Include directives shall be first in the file. In a class
    implementation, include the header file containing the class
    declaration before all other header files, to ensure
    that the header is self-sufficient.

  - Every header file should be self-sufficient in the sense
    that for a header file my_header.h, the following should
    compile without errors:

  ~~~~~~~~~~~~~~~~
  #include "my_header.h"
  ~~~~~~~~~~~~~~~~

  An exception is made for generated files; for example, those
  generated by Yacc and Lex, because it is not possible to
  rewrite the generators to produce "correct" files.
*/

/**
  @page EXAMPLE_SETUP_FOR_CTAGS Example Setup for ctags

  Put this configuration into your ~/.ctags file:
  @verbatim
  --c++-kinds=+p
  --fields=+iaS
  --extra=+q
  --langdef=errmsg
  --regex-errmsg=/^(ER_[A-Z0-9_]+)/\1/
  --langmap=errmsg:(errmsg*.txt),c:+.ic,yacc:+.yy
  @endverbatim
*/

/**
  @page DBUG_TAGS DBUG Tags

  <p>The full documentation of the DBUG library is in files dbug/user.* in the
  MySQL source tree. Here are some of the DBUG tags we now use:</p>

    - enter

      Arguments to the function.

    - exit

      Results from the function.

    - info

      Something that may be interesting.

    - warning

      When something does not go the usual route or may be wrong.

    - error

      When something went wrong.

    - loop

      Write in a loop, that is probably only useful when debugging the loop.
      These should normally be deleted when you are satisfied with the code
      and it has been in real use for a while.

  <br>
  Some tags specific to mysqld, because we want to watch these carefully:

    - trans

      Starting/stopping transactions.

    - quit

      info when mysqld is preparing to die.

    - query

      Print query.

*/
