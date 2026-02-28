/*
   Copyright (c) 2002, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sp_head.h"

#include <stdio.h>
#include <string.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include "my_config.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include <new>
#include <utility>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_alloc.h"
#include "my_bitmap.h"
#include "my_dbug.h"
#include "my_hostname.h"
#include "my_inttypes.h"
#include "my_pointer_arithmetic.h"
#include "my_systime.h"
#include "my_user.h"  // parse_user
#include "mysql/components/services/psi_error_bits.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_error.h"
#include "mysql/psi/mysql_sp.h"
#include "mysql/psi/mysql_statement.h"
#include "mysql_com.h"
#include "prealloced_array.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // *_ACL
#include "sql/binlog.h"
#include "sql/check_stack.h"
#include "sql/dd/dd.h"          // get_dictionary
#include "sql/dd/dictionary.h"  // is_dd_table_access_allowed
#include "sql/derror.h"         // ER_THD
#include "sql/discrete_interval.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/locked_tables_list.h"
#include "sql/log_event.h"  // append_query_string, Query_log_event
#include "sql/mdl.h"
#include "sql/mysqld.h"     // atomic_global_query_id
#include "sql/opt_trace.h"  // opt_trace_disable_etc
#include "sql/protocol.h"
#include "sql/protocol_classic.h"
#include "sql/psi_memory_key.h"
#include "sql/query_options.h"
#include "sql/session_tracker.h"
#include "sql/sp.h"
#include "sql/sp_instr.h"
#include "sql/sp_pcontext.h"
#include "sql/sp_rcontext.h"
#include "sql/sql_base.h"  // close_thread_tables
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_db.h"  // mysql_opt_change_db, mysql_change_db
#include "sql/sql_digest_stream.h"
#include "sql/sql_error.h"
#include "sql/sql_parse.h"  // cleanup_items
#include "sql/sql_profile.h"
#include "sql/sql_show.h"  // append_identifier
#include "sql/thd_raii.h"
#include "sql/thr_malloc.h"
#include "sql/transaction.h"  // trans_commit_stmt
#include "sql/trigger_def.h"
#include "sql_string.h"
#include "template_utils.h"  // pointer_cast
#include "thr_lock.h"

/**
  @page stored_programs Stored Programs

  @section sp_overview Overview

  Stored Programs in general refers to:

  - <tt>PROCEDURE</tt>
  - <tt>FUNCTION</tt>
  - <tt>%TABLE TRIGGER</tt>
  - <tt>EVENT</tt>

  When developing, there are a couple of tools available in the server itself
that are helpful. These tools are only available in builds compiled with
debugging support:

  - <tt>SHOW PROCEDURE CODE</tt>
  - <tt>SHOW FUNCTION CODE</tt>

  The equivalent for triggers or events is not available at this point.

  The internal implementation of Stored Programs in the server
  depends on several components:

  - the storage layer, used to store in the database itself
  a program (hence the name stored program)
  - the internal memory representation of a Stored Program,
  used within the server implementation
  - the SQL parser, used to convert a Stored Program from
  its persistent representation to its internal form
  - a flow analyser, used to optimize the code representing a stored program
  - various caches, used to improve performance by avoiding the need
  to load and parse a stored program at every invocation
  - the Stored Program runtime execution itself,
  which interprets the code of the program and executes its statements

  @section sp_storage Persistent Representation

  Storage of Stored Programs is implemented using either
  tables in the database (in the @c mysql schema), or physical files.

  @subsection sp_storage_sp_sf Stored Procedure and Stored Function Storage

  The table <tt>mysql.proc</tt> contains
  one record per Stored Procedure or Stored Function.
  Note that this table design is a mix of relational
  and non relational (blob) content:

  - Attributes that are part of the interface of a stored procedure
  or function (like its name, return type, etc),
  or that are global to the object (implementation language,
  deterministic properties, security properties, sql mode, etc)
  are stored with a dedicated column in table <tt>mysql.proc</tt>.

  - The body of a stored procedure or function,
  which consists of the original code expressed in SQL,
  including user comments if any, is stored as-is preserving
  the original indentation in blob column 'body'.

  This design choice allows the various attributes to be
  represented in a format that is easy to work with (relational model),
  while allowing a lot of flexibility for the content of the body.

  A minor exception to this is the storage of the parameters
  of a stored procedure or function (which are part of its interface)
  inside the blob column @c param_list (instead of using a child table @c
proc_param).

  Table <tt>mysql.procs_priv</tt> describes privileges granted
  for a given Stored Procedure or Stored Function in table <tt>mysql.proc</tt>.

  The code used to encapsulate database access is:

  - #sp_create_routine()
  - #db_load_routine()
  - #sp_drop_routine()
  - #mysql_routine_grant()
  - #grant_load()
  - #grant_reload()

  @subsection sp_storage_trigger Table Trigger Storage

  Information for a given trigger is stored in the table mysql.triggers
  of the Data Dictionary.

  The code used to encapsulate access is:

  - #Table_trigger_dispatcher::create_trigger()

  - #Table_trigger_dispatcher::check_n_load()

  See the C++ class #Table_trigger_dispatcher in general.

  @warning The current implementation of the storage layer for table triggers
  is considered private to the server,
  and might change without warnings in future releases.

  @subsection sp_storage_event Event Storage

  %Events storage is very similar to Stored Procedure
  and Stored Function storage, and shares the same design.
  Since more attributes are needed to represent an event,
  a different table is used: table <tt>mysql.event</tt>.

  The code used to encapsulate the database access is:

  - #Event_db_repository::create_event()
  - #Event_db_repository::update_event()
  - #Event_db_repository::drop_event()

  See the C++ class #Event_db_repository in general.

  @subsection sp_storage_derived Derived Attributes Storage

  Some critical attributes, such as @c SQL_MODE,
  are explicitly part of the storage format.

  Other attributes, that also impact significantly the behavior
  in general of Stored Programs, can be implicitly derived
  from other properties of the storage layer.
  In particular:

  - The <tt>USE @<database@></tt> in effect for a stored program
  is the schema the stored object belongs to.

  - The statement <tt>DECLARE v CHAR(10)</tt> does not intrinsically convey
  any notion of character set or collation.
  The character set and collation of this local variable,
  in a stored program, derives from the character set and collation
  of the schema the stored object belongs to.

  @section sp_internal Internal Representation

  A Stored Program is represented in memory by two major parts:

  - The code of the stored program, including SQL statements
  and control flow logic (IF, WHILE, ...),

  - A symbol table that describes all the local variables,
  cursors, labels, conditions ... declared in the code.

  Individual instructions of various kind are implemented by all
  the C++ classes that inherit from class #sp_instr.
  The symbol table ('symbol table' is a term used in conjunction
                    with compilers or interpreters,
                    in MySQL the term 'Parsing Context' is used instead)
  is implemented by the C++ class #sp_pcontext.
  A Stored Program as a whole is represented by the C++ class #sp_head,
  which contains the instructions (array #sp_head::m_instructions)
  and the root parsing context (member #sp_head::m_root_parsing_ctx).

  @attention Class #sp_head contains concepts from different areas.
  It represents both what a stored program @em is,
  which is the topic of this section,
  and how a stored program logic <em> is used </em> during runtime
interpretation, which is the subject of other sections.

  @subsection sp_internal_instr Instructions

  Data Definition Language and Data Manipulation Language SQL statements
  are represented as-is, by a single instruction.
  For flow control statements and exception handlers,
  several instructions are used to implement in the low level
  #sp_instr language the semantic of the SQL construct.

  Let's see an example with a stored procedure:

@verbatim
delimiter $$

CREATE PROCEDURE proc_1(x int)
BEGIN
  IF x < 0 THEN
   INSERT INTO t1 VALUES ("negative");
  ELSEIF x = 0 THEN
   INSERT INTO t1 VALUES ("zero");
  ELSE
   INSERT INTO t1 VALUES ("positive");
  END IF;
END$$
@endverbatim

  The resulting code, displayed by <tt>SHOW PROCEDURE CODE</tt>, is:

@verbatim
SHOW PROCEDURE CODE proc_1;
Pos     Instruction
0       jump_if_not 3(7) (x@0 < 0)
1       stmt 5 "INSERT INTO t1 VALUES ("negative")"
2       jump 7
3       jump_if_not 6(7) (x@0 = 0)
4       stmt 5 "INSERT INTO t1 VALUES ("zero")"
5       jump 7
6       stmt 5 "INSERT INTO t1 VALUES ("positive")"
@endverbatim

  Instructions are numbered sequentially.
  Position 0 is the start of the code.
  The position 7 that is one past the last instruction
  in this example represents the end of the code.

  Note that the instruction jump_if_not 3(7) at position 0
  can actually jump to three locations:

  - When the evaluation of the condition "x < 0" is true,
  the next instruction will be position 1 (the "then" branch),

  - When the evaluation of the condition "x < 0" is false,
  the next instruction will be position 3 (the "else" branch),

  - When the evaluation of the condition "x < 0" results in an error,
  and when a continue handler exists for the error,
  the next instruction will be position 7,
  known as the "continuation" destination.

  Now, let's see how exception handlers are represented.
  The following code contains just a very basic handler,
  protecting a BEGIN/END block in the SQL logic:

@verbatim
CREATE PROCEDURE proc_2(x int)
BEGIN
  SELECT "Start";

  INSERT INTO t1 VALUES (1);

  BEGIN
    DECLARE CONTINUE HANDLER FOR SQLEXCEPTION
    BEGIN
      SELECT "Oops";
    END;

    INSERT INTO t1 VALUES (2);
    INSERT INTO t1 VALUES (2);
  END;

  INSERT INTO t1 VALUES (3);
  SELECT "Finish";
END$$
@endverbatim

  The internal instructions for this stored procedure are:

@verbatim
SHOW PROCEDURE CODE proc_2;
Pos     Instruction
0       stmt 0 "SELECT "Start""
1       stmt 5 "INSERT INTO t1 VALUES (1)"
2       hpush_jump 5 1 CONTINUE
3       stmt 0 "SELECT "Oops""
4       hreturn 1
5       stmt 5 "INSERT INTO t1 VALUES (2)"
6       stmt 5 "INSERT INTO t1 VALUES (2)"
7       hpop 1
8       stmt 5 "INSERT INTO t1 VALUES (3)"
9       stmt 0 "SELECT "Finish""
@endverbatim

  Note the flow of control in the code: there is not a single if.
  The couple of @c hpush_jump / @c hpop represent the installation
  and the removal of the exception handler.
  The body of the exception handler starts at position 3,
  whereas the code protected by the handler starts at position 5.
  @c hpush_jump 5 1 means: add a handler for "1" condition (sqlexception),
  where "1" stands for the index of declared conditions in the parsing context,
  and execute the code starting at position "5".

  @subsection sp_internal_pcontext Parsing Context

  A parsing context is a tree of nodes,
  where each node contains symbols (variables, cursors, labels, ...)
  declared locally in the same name visibility scope.

  For example, with the following SQL code:

@verbatim
CREATE PROCEDURE proc_3(x int, y int)
BEGIN
  -- This is the root parsing context
  DECLARE v1 INT;
  DECLARE v2 INT;
  DECLARE v3 INT;

  IF (x > 0) THEN
    BEGIN
      -- This is the child context A
      DECLARE v1 INT;
      DECLARE v4 INT DEFAULT 100;

      set v4:= 1;
      set v1:= x;
    END;
  ELSE
    BEGIN
      -- This is the child context B
      DECLARE v2 INT;
      DECLARE v4 INT DEFAULT 200;

      set v4:= 2;
      set v2:= y;
      set v3:= 3;
    END;
  END IF;

  set v1 := 4;
END$$
@endverbatim

  The parsing contexts match exactly the nesting of BEGIN/END blocks:

  - The root parsing context contains parameters x, y,
  and local variables v1, v2, v3,

  - The BEGIN/END block in the THEN part defines a child parsing
  context (let's call it 'A'), that contains local variables v1 and v4,

  - Likewise, the ELSE block defines a parsing context
  (let's call it 'B') which is a child of the root,
  and contains local variables v2 and v4.

  The total number of symbols is 9: 5 for the root + 2 for A + 2 for B.
  All the symbols are numbered internally (starting at offset 0),
  by walking the parsing context tree in a depth first manner,
  resulting in the following:

  - Root:x --> 0, Root:y --> 1, Root:v1 --> 2, Root:v2 --> 3, Root:v3 --> 4,

  - A:v1 --> 5, A:v4 --> 6,

  - B:v2 --> 7, B:v4 --> 8,

  There is no tool to dump the parsing context tree explicitly.
  However, the internal numbering of symbols is apparent when printing the code:

@verbatim
SHOW PROCEDURE CODE proc_3;
Pos     Instruction
0       set v1@2 NULL
1       set v2@3 NULL
2       set v3@4 NULL
3       jump_if_not 9(14) (x@0 > 0)
4       set v1@5 NULL
5       set v4@6 100
6       set v4@6 1
7       set v1@5 x@0
8       jump 14
9       set v2@7 NULL
10      set v4@8 200
11      set v4@8 2
12      set v2@7 y@1
13      set v3@4 3
14      set v1@2 4
@endverbatim

  The points of interest are that:

  - There are two variables named v1,
  where the variable v1 from block A (represented as v1@5)
  eclipses the variable v1 from the root block (represented as v1@2).

  - There are two variables named v4, which are independent.
  The variable v4 from block A is represented as v4@6,
  whereas the variable v4 from block B is represented as v4@8.

  The parsing context C++ class, #sp_pcontext,
  contains much more information related to each symbol,
  notably data types of variables
  (unfortunately not printable with SHOW PROCEDURE CODE).

  @section sp_parser Stored Program Parser

  There is no “Stored Program Parser” as such,
  there is only one parser in the SQL layer in the server.
  This parser is capable of understanding every SQL statement,
  including statements related to Stored Programs.
  The parser is implemented as an ascendant parser, using bison.
  The source code is located in the file sql/sql_yacc.yy.

  The parts of the parser dedicated more specially to
  Stored Programs are starting at the following rules:

  - <tt>CREATE PROCEDURE</tt> : see rule sp_tail,

  - <tt>CREATE FUNCTION</tt> : see rule sp_tail,

  - <tt>CREATE TRIGGER</tt> : see rule trigger_tail,

  - <tt>CREATE EVENT</tt> : see rule event_tail.

  In every case, the parser reads the SQL text stream that
  represents the code as input, and creates an internal representation
  of the Stored Program as output, with one C++ object of type #sp_head.
  A limiting consequence of this approach is that a stored program
  does not support nesting: it is impossible to embed one
  <tt>CREATE PROCEDURE</tt> into another,
  since the parser currently may only support
  one #sp_head object at a time.

  @subsection sp_parser_structure Parser Structure

  Conceptually, there are many different layers involved during parsing:

  - Lexical analysis (making words or tokens from a character stream),

  - Syntactic analysis (making "sentences" or an abstract syntax tree from the
tokens),

  - Semantic analysis (making sure these sentences do make sense),

  - Code generation (for compilers) or evaluation (for interpreters).

  From the implementation point or view,
  many different concepts from different layers actually collide
  in the same code base, so that the actual code organization is as follows:

  - The lexical analysis is implemented in sql/sql_lex.cc,
  as when parsing regular statements.

  - Syntactic analysis, semantic analysis,
  and code generation -- all of them -- are done at once,
  during parsing of the code.
  From that perspective, the parser behaves as a single pass compiler.
  In other words, both the code and the symbol table
  for the final result are generated on the fly,
  interleaved with syntactic analysis.

  This is both very efficient from a performance point of view,
  but difficult to understand, from a maintenance point of view.

  Let's illustrate for example how the following SQL statement is parsed:

@code
 DECLARE my_cursor CURSOR FOR SELECT col1 FROM t1;
@endcode

  The corresponding part of the grammar in the parser
  for DECLARE CURSOR statements is the following (with annotated line numbers):

@verbatim
[ 1] sp_decl:
[ 2]   DECLARE_SYM ident CURSOR_SYM FOR_SYM sp_cursor_stmt
[ 3]   {
[ 4]     LEX *lex= Lex;
[ 5]     sp_head *sp= lex->sphead;
[ 6]     sp_pcontext *ctx= lex->spcont;
[ 7]     uint offp;
[ 8]     sp_instr_cpush *i;
[ 9]
[10]     if (ctx->find_cursor(&$2, &offp, true))
[11]     {
[12]       my_error(ER_SP_DUP_CURS, MYF(0), $2.str);
[13]       delete $5;
[14]       MYSQL_YYABORT;
[15]     }
[16]     i= new sp_instr_cpush(sp->instructions(), ctx, $5,
[17]                           ctx->current_cursor_count());
[18]     sp->add_instr(i);
[19]     ctx->push_cursor(&$2);
[20]     $$.vars= $$.conds= $$.hndlrs= 0;
[21]     $$.curs= 1;
[22]   }
[23] ;
@endverbatim

  The lines [1], [2] and [23] are bison code
  that express the structure of the grammar.
  These lines belong to the syntactic parsing realm.

  The lines [3] and [22] are bison delimiters
  for the associated action to execute,
  when parsing of the syntax succeeds.
  Everything between lines [3] and [22] is C++ code,
  executed when the parser finds a syntactically
  correct DECLARE %CURSOR statement.

  The lines [4] to [8] could be considered syntactic parsing:
  what the code does is find what is the current
  Stored Program being parsed,
  find the associated part of the syntax tree under construction (#sp_head),
  and find the associated current context
  in the symbol table (#sp_pcontext).

  Note that there is some black magic here:
  since we are still currently parsing the content
  of a Stored Program (the DECLARE %CURSOR statement),
  the final “syntax” tree for the Stored Program (#sp_head)
  is not supposed to exist yet. The reason the #sp_head object
  is already available is that the actions in the
  <tt>CREATE PROCEDURE</tt>, <tt>CREATE FUNCTION</tt>, <tt>CREATE TRIGGER</tt>,
  or <tt>CREATE EVENT</tt> are implemented as a descendant parser
  (it created an empty #sp_head object first, filling the content later).
  Mixing code that way (descendant actions with ascendant parsing)
  is extremely sensitive to changes.

  The line [10] is a semantic check.
  The statement might be syntactically correct (it parsed),
  but to be semantically correct,
  the name or the cursor must be unique in the symbol table.

  Line [12] is reporting a semantic error back to the client (duplicate cursor).
  The code at line [14] forces the syntactic parser (bison) to abort.

  By line [16], we have verified that the code is syntactically valid,
  and semantically valid: it's now time for code generation,
  implemented by creating a new #sp_instr_cpush to represent
  the cursor in the compiled code.
  Note that variable allocation is done on the fly,
  by looking up the current cursor count in the symbol table
  (#sp_pcontext::current_cursor_count()).

  Line [18] adds the generated code to the object
  representing the stored program (code generation).

  Line [19] maintains the symbol table (semantic parsing)
  by adding the new cursor in the current local context.

  Lines [20] and [21] return to bison a fragment of
  a fake syntax tree, indicating that one cursor was found.

  By looking at the complete implementation of this action in bison,
  one should note that the target code was generated,
  the symbol table for the Stored Program was looked up and updated,
  while at no point in time a syntax node was even created.
  Note that the #sp_instr_cpush object should really be considered generated
code: the fact that there is a one-to-one correspondence with the syntax is
incidental.

  @subsection sp_parser_codegen Single-Pass Code Generation

  All the code generated by the parser is emitted in a single pass.
  For example, consider the following SQL logic:

@verbatim
CREATE FUNCTION func_4(i int)
RETURNS CHAR(10)
BEGIN
  DECLARE str CHAR(10);

  CASE i
    WHEN 1 THEN SET str="1";
    WHEN 2 THEN SET str="2";
    WHEN 3 THEN SET str="3";
    ELSE SET str="unknown";
  END CASE;

  RETURN str;
END$$
@endverbatim

  The compiled program for this Stored Function is:

@verbatim
SHOW FUNCTION CODE func_4;
Pos     Instruction
0       set str@1 NULL
1       set_case_expr (12) 0 i@0
2       jump_if_not 5(12) (case_expr@0 = 1)
3       set str@1 _latin1'1'
4       jump 12
5       jump_if_not 8(12) (case_expr@0 = 2)
6       set str@1 _latin1'2'
7       jump 12
8       jump_if_not 11(12) (case_expr@0 = 3)
9       set str@1 _latin1'3'
10      jump 12
11      set str@1 _latin1'unknown'
12      freturn 254 str@1
@endverbatim

  Note the instruction at position 4: jump 12.
  How can the compiler generate this instruction in a single pass,
  when the destination (12) is not known yet ?
  This instruction is a forward jump.
  What happens during code generation is that,
  by the time the compiler has generated the code
  for positions [0] to [11], the generated code looks like this:

@verbatim
Pos     Instruction
0       set str@1 NULL
1       set_case_expr ( ?? ) 0 i@0
2       jump_if_not 5( ?? ) (case_expr@0 = 1)
3       set str@1 _latin1'1'
4       jump ??
5       jump_if_not 8( ?? ) (case_expr@0 = 2)
6       set str@1 _latin1'2'
7       jump ??
8       jump_if_not 11( ?? ) (case_expr@0 = 3)
9       set str@1 _latin1'3'
10      jump ??
11      set str@1 _latin1'unknown'
...
@endverbatim

  The final destination of the label for the END CASE is not known yet,
  and the list of all the instructions (1, 2, 4, 5, 7, 8 and 10)
  that need to point to this unknown destination (represented as ??)
  is maintained in a temporary structure used during code generation only.
  This structure is called the context back patch list.

  When the destination label is finally resolved to a destination (12),
  all the instructions pointing to that label,
  which have been already generated (but with a bogus destination)
  are back patched to point to the correct location.
  See the comments marked @c BACKPATCH in the code for more details.

  As a side note, this generated code also shows that
  some temporary variables can be generated implicitly,
  such as the operand of the CASE expression, labeled case_expr@0.

  @attention Numbering of case expressions in the symbol table
  uses a different name space than variables,
  so that case_expr@0 and i@0 are two different variables,
  even when both internally numbered with offset zero.

  @section sp_optimizer Flow Analysis Optimizations

  After code is generated, the low level sp_instr instructions are optimized.
  The optimization focuses on two areas:

  - Dead code removal,

  - Jump shortcut resolution.

  These two optimizations are performed together,
  as they both are a problem involving flow analysis
  in the graph that represents the generated code.

  The code that implements these optimizations is #sp_head::optimize().

  @attention Do not confuse #sp_head::optimize()
  with the component named the optimizer,
  as they are very different.
  The former is specific to Stored Programs,
  and focuses on improving the flow of statements,
  whereas the latter is general to queries,
  and focuses on finding the best execution plan when executing a single
statement. For the optimizer, see Optimization.

  The (Stored Program) optimizer is invoked from only one place,
  in the following code:

@code
db_load_routine(..., sp_head **sphp, ...)
{
  ...
  (*sphp)->optimize();
  ...
}
@endcode

  @note By disabling the call to #sp_head::optimize()
  and recompiling the code,
  SHOW PROCEDURE CODE will display the code before flow optimization.

  @attention When investigating issues related to this area,
  you may want to use a @c DBUG_EXECUTE_IF
  to avoid recompiling the server with or without
  flow optimization every time.
  Be careful to shutdown and restart the server with or without
  the call to #sp_head::optimize() for each test,
  or you will find that caching of a Stored Program code does interfere.

  @subsection sp_optimizer_dead Dead Code Removal

  'Dead code' is also known as 'unreachable code':
  code that cannot possibly be executed,
  because no path in the logic leads to it.

  For example, consider the following SQL code:

@verbatim
CREATE PROCEDURE proc_5()
BEGIN
  DECLARE i INT DEFAULT 0;

  again:
  WHILE TRUE DO
    BEGIN
      set i:= i+1;

      SELECT "This code is alive";

      IF (i = 100) THEN
        LEAVE again;
      END IF;

      ITERATE again;

      SELECT "This code is dead";
    END;
  END WHILE;
END$$
@endverbatim

  Before flow optimization, the compiled code is:

@verbatim
SHOW PROCEDURE CODE proc_5;
Pos     Instruction
0       set i@0 0
1       jump_if_not 10(10) 1
2       set i@0 (i@0 + 1)
3       stmt 0 "SELECT "This code is alive""
4       jump_if_not 7(7) (i@0 = 100)
5       jump 10
6       jump 7
7       jump 1
8       stmt 0 "SELECT "This code is dead""
9       jump 1
@endverbatim

  Note the instruction at position 8:
  the previous instruction is an unconditional jump,
  so the flow of control can never reach 8 by coming from 7.
  Because there exists no jump in the entire code that leads to 8 either,
  the instruction at 8 is unreachable.
  By looking further in the flow,
  because 8 is unreachable and there are no jumps to position 9,
  the instruction at position 9 is also unreachable.

  The instruction at position 6 is also unreachable,
  for a similar reason: the THEN part of the if contains a jump,
  due to the statement LEAVE again;,
  so that the code never executes the jump generated by the compiler
  to go from the end of the THEN block to the statement following the IF.

  After detecting all the unreachable instructions,
  and simplifying the code,
  the result after flow optimization is:

@verbatim
SHOW PROCEDURE CODE proc_5;
Pos     Instruction
0       set i@0 0
1       jump_if_not 10(10) 1
2       set i@0 (i@0 + 1)
3       stmt 0 "SELECT "This code is alive""
4       jump_if_not 1(1) (i@0 = 100)
5       jump 10
@endverbatim

  The flow optimizer is good at detecting most of the dead code,
  but has limitations.
  For example, coding in SQL IF FALSE THEN ... END IF;
  leads to code that can never be executed,
  but since the flow optimizer does neither propagate constants
  nor consider impossible conditional jumps, this code will not be removed.

  The goal of the flow optimizer is mostly to perform
  simple local optimizations with a low cost.
  It's not a fully featured code optimizer,
  and does not guard against poor SQL.

  @subsection sp_optimizer_jump Jump Shortcut Resolution

  The term jump shortcut refers to the following optimization:
  when instruction A is a jump (conditional or not)
  that goes to position B,
  and when B is an unconditional jump to position C,
  the code can be changed so that A can jump to C directly,
  taking a shortcut to avoid the unnecessary B.
  Consider the following SQL code:

@verbatim
CREATE PROCEDURE proc_6(x int, y int, z int)
BEGIN
  SELECT "Start";

  IF (x > 0)
  THEN
    BEGIN
      SELECT "x looks ok";
      IF (y > 0)
      THEN
        BEGIN
          SELECT "so does y";
          IF (z > 0)
          THEN
            SELECT "even z is fine";
          ELSE
            SELECT "bad z";
          END IF;
        END;
      ELSE
        SELECT "bad y";
      END IF;
    END;
  ELSE
    SELECT "bad x";
  END IF;

  SELECT "Finish";
END$$
@endverbatim

  Before flow optimization, the compiled code is:

@verbatim
SHOW PROCEDURE CODE proc_6;
Pos     Instruction
0       stmt 0 "SELECT "Start""
1       jump_if_not 12(13) (x@0 > 0)
2       stmt 0 "SELECT "x looks ok""
3       jump_if_not 10(11) (y@1 > 0)
4       stmt 0 "SELECT "so does y""
5       jump_if_not 8(9) (z@2 > 0)
6       stmt 0 "SELECT "even z is fine""
7       jump 9
8       stmt 0 "SELECT "bad z""
9       jump 11
10      stmt 0 "SELECT "bad y""
11      jump 13
12      stmt 0 "SELECT "bad x""
13      stmt 0 "SELECT "Finish""
@endverbatim

  Note the jump 9 at position 7:
  since the instruction at position 9 is jump 11,
  the code at position 7 can be simplified to jump 11.
  The optimization is also recursive:
  since the instruction 11 is jump 13,
  the final jump destination for the instruction at position 7 is jump 13.
  Conditional jumps are optimized also,
  so that the instruction 5: jump_if_not 8(9) can be optimized to jump_if_not
8(13).

  After flow optimization, the compiled code is:

@verbatim
SHOW PROCEDURE CODE proc_6;
Pos     Instruction
0       stmt 0 "SELECT "Start""
1       jump_if_not 12(13) (x@0 > 0)
2       stmt 0 "SELECT "x looks ok""
3       jump_if_not 10(13) (y@1 > 0)
4       stmt 0 "SELECT "so does y""
5       jump_if_not 8(13) (z@2 > 0)
6       stmt 0 "SELECT "even z is fine""
7       jump 13
8       stmt 0 "SELECT "bad z""
9       jump 13
10      stmt 0 "SELECT "bad y""
11      jump 13
12      stmt 0 "SELECT "bad x""
13      stmt 0 "SELECT "Finish""
@endverbatim

  Note the differences with every jump instruction.

  @attention For clarity, this example has been designed
  to not involve dead code.
  Note that in general, an instruction that was reachable
  before taking a shortcut might become unreachable after the shortcut,
  so that the optimizations for jump shortcuts
  and dead code are tightly intertwined.

  @section sp_cache Stored Program Caches

  The goal of the Stored Program cache is to keep
  a parsed sp_head in memory, for future reuse. Reuse means:

  - To be able to execute concurrently the same
  Stored Program in different THD threads,

  - To be able to execute the same Stored Program
  multiple times (for recursive calls) in the same THD thread.

  To achieve this, the implementation of #sp_head must be
  both thread-safe and stateless.
  Unfortunately, it is neither:

  - The class #sp_head is composed of #sp_instr instructions to represent the
code, and these instructions in turn depend on Item objects, used to represent
the internal structure of a statement. The various C++ Item classes are not
currently thread-safe, since the evaluation of an Item at runtime involves
methods like Item::fix_fields(), which modify the internal state of items,
  making them impossible to safely evaluate concurrently.

  - The class #sp_head itself contains attributes that describe
  the SQL logic of a Stored Program (which are safe to share),
  mixed with attributes that relate to the evaluation
  of this logic in a given instance to a Stored Program call
  (mostly the MEM_ROOT memory pool used during execution),
  which by definition cannot be shared.

  The consequence of these restrictions is less than optimal code.
  What is currently implemented in the server
  is detailed in the following subsections, to help maintenance.

  @attention Needless to say, the current implementation
  of Stored Program caching is by no mean final,
  and could be re factored in future releases.

  @subsection sp_cache_sp Stored Procedure Cache

  The PROCEDURE cache is maintained on a per thread basis,
  in #THD::sp_proc_cache.

  The function used to lookup the cache is #sp_find_routine.
  It relies on the C++ class #sp_cache for the low level implementation.

  There is a global mechanism to invalidate all the caches
  of all the THD threads at once,
  implemented with the variable #atomic_Cversion in file sp_cache.cc,
  which is incremented by function #sp_cache_invalidate().
  This global invalidation is used when the server executes
  DROP PROCEDURE or ALTER PROCEDURE statements.

  Each entry in the cache is keyed by name,
  and consists of a linked list of stored procedure instances
  which are all duplicates of the same object.
  The reason for the list is recursion,
  when the runtime needs to evaluate several calls
  to the same procedure at once.

  The runtime behavior of this caching mechanism
  has some limitations, and in particular:

  - Each #THD has its own cache, so each separate client connection
  to the server uses its own cache.
  Multiple client connections calling the same Stored Procedure
  will cause the parser to be invoked multiple times,
  and memory to be consumed multiple times.

  - If a given client constantly opens and closes
  a new connection to the server,
  and invokes Stored Procedures,
  the cache will be always empty,
  causing excessive parsing of used stored procedures on every invocation.

  - If a given client constantly keeps an existing connection
  to the server for a long time, and invokes Stored Procedures,
  the cache size will grow, consuming and retaining memory.
  In other words, memory limits or expulsion of cold members
  of the stored procedure cache is not implemented.

  - Calling #sp_cache_invalidate() does not reclaim the cache memory.
  This memory will be reclaimed only if a Stored Procedure
  is looked up in the cache again, causing the cache to flush.

  @subsection sp_cache_sf Stored Function Cache

  The FUNCTION cache is implemented exactly
  like the PROCEDURE cache,
  in the thread member in #THD::sp_func_cache.

  Note that because #THD::sp_proc_cache and #THD::sp_func_cache are
  both invalidated based on the same #atomic_Cversion counter,
  executing DROP PROCEDURE happens to invalidate the FUNCTION cache as well,
  while DROP FUNCTION also invalidates the PROCEDURE cache.
  In practice, this has no consequences since DDL statements
  like this are not executed typically while an application is running,
  only when it is deployed.

  @subsection sp_cache_trg Table Trigger Cache

  For table triggers, all the triggers that relate to
  a given table are grouped in the C++ class #Table_trigger_dispatcher,
  which in particular contains the member sp_head
*bodies[TRG_EVENT_MAX][TRG_ACTION_MAX].

  Note that at most one trigger per event (BEFORE, AFTER)
  and per action (INSERT, UPDATE, DELETE) can be defined currently.

  The #Table_trigger_dispatcher itself is a part of struct #TABLE.

  As a result, each table trigger body is duplicated
  in each table handle, which is necessary to properly evaluate them.
  #TABLE handles are globally cached and reused across threads,
  so the table triggers are effectively reused across different
  clients connections manipulating the same physical table.

  @subsection sp_cache_evt Events and Caching

  For events, the #sp_head object that represents the body
  of an EVENT is part of the C++ class #Event_parse_data.

  There is no caching of #sp_head for multiple scheduling of an event.
  The method #Event_job_data::execute() invokes the parser
  every time an event is executed.

  @section sp_execution Stored Program Execution

  Executing a Stored Program consists of interpreting
  the low level #sp_instr code.
  The runtime interpreter itself is implemented
  in the method #sp_head::execute().
  Wrappers for different kinds of Stored Programs
  are implemented in the following methods:

  - @c PROCEDURE : see #sp_head::execute_procedure(),

  - @c FUNCTION : see #sp_head::execute_function(),

  - @c TRIGGER : see #sp_head::execute_trigger(),

  - @c EVENT : see #Event_job_data::execute().

  @subsection sp_exc_rcont Runtime Context

  An interpretor needs to be able to represent the state
  of the SQL program being executed:
  this is the role of the C++ class #sp_rcontext, or runtime context.

  @subsubsection sp_exec_rcont_var Local Variables

  Values of local variables in an SQL Stored Program
  are stored within the #sp_rcontext.
  When the code enters a new scope,
  the sp_instr contains explicit statements to initialize
  the local variable DEFAULT value, if any.
  Since initialization of values is done in the code,
  and since no logic needs to be executed
  when an SQL variable goes out of scope,
  space allocation to represent the data does
  not need to follow the nesting of BEGIN/END blocks during runtime.

  Another important point regarding the representation
  of local SQL variables is that, conceptually,
  a local variable can be considered to be an SQL table
  with a single column (of the variable type),
  with a single row (to represent the value).

  As a result, all the local variables of a Stored Program
  are represented by a row in a table internally.
  For example, consider the following SQL code:

@verbatim
CREATE PROCEDURE proc_7(x int)
BEGIN
  DECLARE v1 INT;
  DECLARE v2 VARCHAR(10);
  DECLARE v3 TEXT;

  IF (x > 0) THEN
    BEGIN
      DECLARE v4 BLOB;
      DECLARE v5 VARCHAR(20);
    END;
  ELSE
    BEGIN
      DECLARE v6 DECIMAL(10, 2);
      DECLARE v7 BIGINT;
    END;
  END IF;
END$$
@endverbatim

  Internally, a temporary table is created, with the following structure:

@verbatim
CREATE TEMPORARY TABLE `proc_7_vars` (
  `v1` int(11) DEFAULT NULL,
  `v2` varchar(10) DEFAULT NULL,
  `v3` text,
  `v4` blob,
  `v5` varchar(20) DEFAULT NULL,
  `v6` decimal(10,2) DEFAULT NULL,
  `v7` bigint(20) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1
@endverbatim

  The real name of the table and the columns are purely internal,
  and the table is not accessible to regular statements
  for DDL or DML operations:
  proc_7_vars and v1 ... v7 are just a notation used in this example.
  The #TABLE handle that implements all the local variable storage
  is the member #sp_rcontext::m_var_table

  Inside a statement, local variables in a Stored Program
  are represented by the dedicated C++ class #Item_splocal.
  #Item_splocal really is a proxy exposing the interface needed to support
#Item, which delegates to the underlying #sp_rcontext for reading or writing
local variable values. The coupling between #Item_splocal and #sp_rcontext is
based on #Item_splocal::m_var_idx, which is the variable index in the symbol
table computed by the parser, and maintained in #sp_pcontext.

  @subsubsection sp_exec_rcont_cursor Cursors

  Unlike local variables,
  some action is needed in the interpreter
  when a CURSOR goes out of scope:
  the cursor must be closed,
  to prevent leaks of the underlying TABLE resources.

  As a result, cursor allocation
  (and really, deallocation so they can be properly closed)
  needs to follow tightly the BEGIN-END block structure of the code,
  so a stack is used,
  implemented by #sp_rcontext::m_cstack and #sp_rcontext::m_ccount.

  @subsubsection sp_exec_rcont_case Case Expressions

  For CASE expressions,
  temporary variables are generated automatically.
  Like CURSOR, there are some constraints that prevent
  treating these special local variables like regular local variables.

  The difficulty with CASE is that the real type
  of the expression is only known when the case statement is executed,
  so that allocating space in a statically computed TABLE is not practical.
  For example, CASE (SELECT col1 FROM t1 WHERE ...)
  is a case expression that involves a single row subselect.
  During parsing, the table might not even exists,
  so evaluating the type of col1 is impossible.
  Creation of the table can be delayed until execution,
  with statements like CREATE TEMPORARY TABLE.

  Instead, an array of Item * is used,
  implemented by #sp_rcontext::m_case_expr_holders.
  The size of the array is static (it's the total number of cases),
  but the content of each element is dynamic
  (to account for the type of the case expression).

  @attention Note the wording used here:
  “static” means something that can be evaluated when compiling the code,
  in the parser,
  whereas “dynamic” means something that can be evaluated
  only when interpreting the code, during runtime.
  Of course, from a C++ coding point of view, everything is dynamic.

  Inside a CASE statement, temporary local variables
  in a Stored Program are represented by the dedicated C++ class
#Item_case_expr. The class #Item_case_expr is also a proxy, similar in nature to
#Item_splocal, and delegates to #sp_rcontext for accessing the underlying case
expression value. The coupling between #Item_case_expr and #sp_rcontext is based
on #Item_case_expr::m_case_expr_id, which is the case expression index in the
symbol table (see #sp_pcontext).

  @subsubsection sp_exec_rcont_handler Exception Handlers

  @todo Update the exception handler doc for SIGNAL, RESIGNAL and GET
DIAGNOSTICS.

  When the code enters a block of logic guarded by an SQL exception handler,
  the state or the runtime context in the interpreter changes,
  to represent this fact.
  The state change is not apparent immediately,
  it will only become apparent if an exception is raised.
  The internal runtime state of the engine also changes
  when the code leaves a block that contains an exception handler.

  How exception handlers work during runtime is the subject
  of another section (“Exception Handling”).
  What is described here is the state maintained internally,
  to represent which HANDLER is currently “active”,
  and what CONDITION is protected against.

  The SQL precedence rules for HANDLER dictates that
  the last installed (inner most) handler is always considered first,
  so the natural structure to represent what handler is active is a stack,
  implemented by #sp_rcontext::m_visible_handlers.

  In addition, some extra information is required for
  CONTINUE handlers: the “address” in the code,
  or instruction pointer in the #sp_instr array,
  of where to resume execution when the handler returns.
  This data is maintained in #sp_rcontext::m_activated_handlers,
  which again is a stack because exception handlers can be nested
  (exceptions can be raised and trapped during the execution
   of the body of an exception handler, too).

  @subsection sp_exec_instr Executing One Instruction

  Executing an instruction consists of calling
  the virtual method #sp_instr::execute(),
  which is implemented for each instruction.

  For instructions that can be executed directly,
  and don't depend on the evaluation of
  a general SQL statement or expression,
  the execution is very simple.
  See for example #sp_instr_jump::execute(),
  #sp_instr_hpush_jump::execute() or #sp_instr_hpop::execute().
  In all cases, the implementation of the execute()
  method is purely internal to the runtime interpreter.

  For instructions that need to evaluate a general expression,
  like #sp_instr_jump_if_not::execute(),
  or general instructions that need to execute an SQL statement,
  such as #sp_instr_stmt::execute(), things are more complex.
  The implementation needs to leverage the existing code
  that is already capable of evaluating an expression or executing a query,
  and is implemented by the function #mysql_execute_command().

  The function #mysql_execute_command(),
  for historical reasons (it was implemented before Stored Programs),
  is mostly designed to consume directly the result of the parser,
  which is passed as input in #THD::lex.

  To comply with this interface,
  the runtime for stored program has to provide a THD::lex structure
  before executing each instruction,
  to prepare an execution environment which looks
  as if the statement to execute was just parsed.
  Dealing with the existing interface for re-entrant
  execution of SQL statements is the role of the C++ class #sp_lex_instr.
  The wrapper method to used to execute instructions
  is #sp_lex_instr::reset_lex_and_exec_core(),
  which ultimately invokes the #sp_lex_instr::exec_core() instructions
implementation.

  @subsection sp_exec_flow Flow Control

  Instructions are numbered sequentially,
  and the current position in the code is represented
  by an “instruction pointer”, which is just an integer.
  In the main execution loop in #sp_head::execute(),
  this instruction pointer is represented by the local variable ip.

  When executing each instruction,
  the method #sp_head::execute() is also responsible to
  return the address of the next instruction to execute.
  Most of the time,
  this corresponds to the “next” instruction (implemented by m_ip+1),
  except for absolute jumps (see #sp_instr_jump::execute())
  or conditional jumps (see #sp_instr_jump_if_not::execute()).

  @subsection sp_exec_handler Exception Handling

  When the code enters a block protected by a HANDLER,
  the execution leads to #sp_instr_hpush_jump::execute(),
  which installs the exception handler in the runtime handler stack,
  by calling #sp_rcontext::push_handler().

  In a similar way, when the code leaves a block protected by a HANDLER,
  #sp_instr_hpop::execute() removes the handlers installed
  by the matching #sp_instr_hpush_jump,
  by calling #sp_rcontext::pop_handlers().

  During the execution of any statement,
  different CONDITION can be raised at runtime,
  which are reported by the implementation of each statement
  by calling push_warning(), my_error() or similar functions.
  All these entry points ultimately leads
  to the error handler hook callback function
  implemented by error_handler_hook in mysys/my_error.c.
  In case of the server itself, this hook points to the function
#my_message_sql().

  Under normal circumstances,
  my_message_sql() just reports a warning or an error
  to the client application, and for errors causes the query to abort.

  When executing a stored program,
  #THD::sp_runtime_ctx points to the runtime context
  of the program currently executed.
  When a HANDLER is active, the runtime context contains
  in its handler stack the list of all the CONDITIONs currently trapped,
  giving a chance to the call to #sp_rcontext::handle_sql_condition() to
intercept error handling.

  If the condition reported does not match
  any of the conditions for which an exception handler is active,
  #sp_rcontext::handle_sql_condition() returns false,
  and #my_message_sql() raises the error or warning as usual.

  When the condition reported does match an active HANDLER,
  that handler is called, but the technical nature of this call is special:
  the call is asynchronous.
  Instead of invoking the exception handler directly,
  #sp_rcontext::handle_sql_condition() marks which exception handler is to be
called, by saving the activation on #sp_rcontext::m_activated_handlers, and then
returns true, so that #my_message_sql() returns without reporting anything: at
this point, the error condition has been totally masked, except for the fact
that #sp_rcontext::m_activated_handlers is set.

  Once #my_message_sql() returns,
  the implementation of a given statement continues,
  either by proceeding if only a warning was reported,
  or by aborting the current execution if an error was raised.
  The execution of code in the server will eventually
  return from the implementation of a statement,
  and return from the call to #sp_instr::execute() for that statement,
  returning control to the loop located in #sp_head::execute().
  Note that during the execution of the code that follows a call to
#my_message_sql(), error conditions are propagated in the call stack though the
function's return value. It is transparent to the implementation of statements
in general whether an exception was caught by an error handler.

  After an instruction is executed in #sp_head::execute(),
  the main interpreter loop checks for any pending exception handler code to
call, by checking the thd error status. If an exception was caught,
  #sp_rcontext::handle_sql_condition() is invoked.

  In case of CONTINUE HANDLER,
  the instruction to return to after the handler code is executed
  needs to be saved in the runtime context.
  Finding the continuation destination is accomplished
  by the call to #sp_instr::get_cont_dest() for the current instruction,
  whereas preserving this destination is done
  with a push on #sp_rcontext::m_activated_handlers.
  The matching call to #sp_rcontext::pop_handler_frame(),
  which is executed when the exception handler is done,
  is located in #sp_instr_hreturn::execute().

  @attention To integrate properly with exception handling in general,
  the code should avoid testing for thd->net.report_error,
  or worse inspecting the content of the error stack (displayed by SHOW ERRORS),
  because doing this actually assumes
  not only that an error was raised,
  but also that it was not caught.
  Instead, the proper way to implement error handling in the server
  is to return error status values and check for them.

  @subsection sp_exec_nest Call Nesting

  In the following example,
  the Stored Procedure proc_1 makes a nested call to proc_2.

@verbatim
CREATE TABLE my_debug(
  seq int NOT NULL AUTO_INCREMENT,
  msg varchar(80),
  PRIMARY KEY(seq)
);

delimiter $$
    CREATE PROCEDURE proc_1()
BEGIN
  INSERT INTO my_debug(msg) VALUES ("entering p1");
  CALL proc_2();
  INSERT INTO my_debug(msg) VALUES ("leaving p1");
END$$

CREATE PROCEDURE proc_2()
BEGIN
  INSERT INTO my_debug(msg) VALUES ("inside p2");
END$$

delimiter ;
    CALL proc_1();
@endverbatim

  @note We do not have a debugger,
  so this is old school printf-like debugging into a table.

  By setting a breakpoint in #Sql_cmd_insert_values::execute_inner in
  the server, the current thread stack at the first insert will look like this:

  @todo Refresh the stack

@verbatim
#0  mysql_insert () at sql_insert.cc:351
#1  in mysql_execute_command () at sql_parse.cc:2643
#2  in sp_instr_stmt::exec_core () at sp_head.cc:2609
#3  in sp_lex_keeper::reset_lex_and_exec_core () at sp_head.cc:2455
#4  in sp_instr_stmt::execute () at sp_head.cc:2560
#5  in sp_head::execute () at sp_head.cc:1077
#6  in sp_head::execute_procedure () at sp_head.cc:1726
#7  in mysql_execute_command () at sql_parse.cc:3807
#8  in mysql_parse () at sql_parse.cc:5274
#9  in dispatch_command () at sql_parse.cc:896
#10 in do_command () at sql_parse.cc:662
#11 in handle_one_connection () at sql_connect.cc:1089
#12 in start_thread () from /lib/libpthread.so.0
#13 in clone () from /lib/libc.so.6
@endverbatim

  By the time the second INSERT is executed, the stack will look like this:

  @todo Refresh the stack

@verbatim
#0  mysql_insert () at sql_insert.cc:351
#1  in mysql_execute_command () at sql_parse.cc:2643
#2  in sp_instr_stmt::exec_core () at sp_head.cc:2609
#3  in sp_lex_keeper::reset_lex_and_exec_core () at sp_head.cc:2455
#4  in sp_instr_stmt::execute () at sp_head.cc:2560
#5  in sp_head::execute () at sp_head.cc:1077
#6  in sp_head::execute_procedure () at sp_head.cc:1726
#7  in mysql_execute_command () at sql_parse.cc:3807
#8  in sp_instr_stmt::exec_core () at sp_head.cc:2609
#9  in sp_lex_keeper::reset_lex_and_exec_core () at sp_head.cc:2455
#10 in sp_instr_stmt::execute () at sp_head.cc:2560
#11 in sp_head::execute () at sp_head.cc:1077
#12 in sp_head::execute_procedure () at sp_head.cc:1726
#13 in mysql_execute_command () at sql_parse.cc:3807
#14 in mysql_parse () at sql_parse.cc:5274
#15 in dispatch_command () at sql_parse.cc:896
#16 in do_command () at sql_parse.cc:662
#17 in handle_one_connection () at sql_connect.cc:1089
#18 in start_thread () from /lib/libpthread.so.0
#19 in clone () from /lib/libc.so.6
@endverbatim

  In this stack trace,
  #sp_head::execute_procedure() at #12 corresponds to CALL proc_1();,
  whereas #sp_head::execute_procedure() at #6 corresponds to CALL proc_2();.
  In other words,
  recursive calls in the user SQL code are implemented
  by performing matching recursive calls in the system C++ code (the server).

  This is actually a severe limitation of the implementation,
  which causes problems for the following reasons:

  - User logic can be arbitrarily nested,
  with a long chain of Stored Programs calling other Stored Programs.
  The total depth of calls can be greater than one would expect,
  especially considering that a VIEW can invoke a FUNCTION,
  and that a TRIGGER can also invoke other PROCEDURE,
  FUNCTION, or TRIGGER objects.

  - The amount of memory that can be consumed in the stack
  for a thread is not infinite.
  In fact, it's quite limited because
  {MAX NUMBER OF THREADS} * {MAX THREAD STACK} = {TOTAL STACK}.
  Note the catch in the equation here: @em MAX thread stack,
  which is dependent on the nesting of stored program in the user SQL code,
  for the worst case.
  When MySQL does not use a thread pool and uses a @em big number of threads,
  this can be a problem affecting scalability.

  - As a result,
  the Stored Program interpreter has to protect itself against stack overflow.
  This is implemented by #check_stack_overrun()

  What should be implemented instead,
  is representing the user SQL stack on the C++ heap,
  and have the interpreter loop instead of making recursive calls.

  There are also other good reasons to use the heap.
  For example, for error reporting,
  the current implementation has no way to tell
  that proc_2 was called from proc_1,
  since this data is not available to the code; it's hidden in the C++ stack.

  Nesting calls also has some impact on SQL exception handlers.
  The member #THD::sp_runtime_ctx for the current thread
  is not pointing to a single #sp_rcontext,
  but to a stack of runtime contexts.

  With the example used,
  when the code is executing proc_1,
  #THD::sp_runtime_ctx points to the runtime context for proc_1.
  When the code is inside proc_2,
  the current thread #THD::sp_runtime_ctx points to #sp_rcontext{proc_2}.
  This pointer is saved and restored during each stored program execution.
*/

#ifdef HAVE_PSI_INTERFACE
void init_sp_psi_keys() {
  const char *category = "sp";

  mysql_statement_register(category, &sp_instr_stmt::psi_info, 1);
  mysql_statement_register(category, &sp_instr_set::psi_info, 1);
  mysql_statement_register(category, &sp_instr_set_trigger_field::psi_info, 1);
  mysql_statement_register(category, &sp_instr_jump::psi_info, 1);
  mysql_statement_register(category, &sp_instr_jump_if_not::psi_info, 1);
  mysql_statement_register(category, &sp_instr_freturn::psi_info, 1);
  mysql_statement_register(category, &sp_instr_hpush_jump::psi_info, 1);
  mysql_statement_register(category, &sp_instr_hpop::psi_info, 1);
  mysql_statement_register(category, &sp_instr_hreturn::psi_info, 1);
  mysql_statement_register(category, &sp_instr_cpush::psi_info, 1);
  mysql_statement_register(category, &sp_instr_cpop::psi_info, 1);
  mysql_statement_register(category, &sp_instr_copen::psi_info, 1);
  mysql_statement_register(category, &sp_instr_cclose::psi_info, 1);
  mysql_statement_register(category, &sp_instr_cfetch::psi_info, 1);
  mysql_statement_register(category, &sp_instr_error::psi_info, 1);
  mysql_statement_register(category, &sp_instr_set_case_expr::psi_info, 1);
}
#endif

/**
  SP_TABLE represents all instances of one table in an optimized multi-set of
  tables used by a stored program.
*/
struct SP_TABLE {
  /*
    Multi-set key:
      db_name\0table_name\0alias\0 - for normal tables
      db_name\0table_name\0        - for temporary tables
    Note that in both cases we don't take last '\0' into account when
    we count length of key.
  */
  LEX_STRING qname;
  size_t db_length, table_name_length;
  bool temp;               /* true if corresponds to a temporary table */
  thr_lock_type lock_type; /* lock type used for prelocking */
  uint lock_count;
  uint query_lock_count;
  uint8 trg_event_map;
};

///////////////////////////////////////////////////////////////////////////
// Static function implementations.
///////////////////////////////////////////////////////////////////////////

/**
  Helper function which operates on a THD object to set the query start_time to
  the current time.

  @param thd  Thread context.
*/
static void reset_start_time_for_sp(THD *thd) {
  if (thd->in_sub_stmt) return;

  /*
    First investigate if there is a cached time stamp
  */
  if (thd->user_time.tv_sec || thd->user_time.tv_usec)
    thd->start_time = thd->user_time;
  else
    my_micro_time_to_timeval(my_micro_time(), &thd->start_time);
}

/**
  Merge contents of two hashes representing sets of routines used
  by statements or by other routines.

  @param dst   hash to which elements should be added
  @param src   hash from which elements merged

  @note
    This procedure won't create new Sroutine_hash_entry objects,
    instead it will simply add elements from source to destination
    hash. Thus time of life of elements in destination hash becomes
    dependant on time of life of elements from source hash. It also
    won't touch lists linking elements in source and destination
    hashes.
*/

static void sp_update_sp_used_routines(
    malloc_unordered_map<std::string, Sroutine_hash_entry *> *dst,
    const malloc_unordered_map<std::string, Sroutine_hash_entry *> &src) {
  for (const auto &key_and_value : src) dst->insert(key_and_value);
}

///////////////////////////////////////////////////////////////////////////
// sp_name implementation.
///////////////////////////////////////////////////////////////////////////

/**
  Create temporary sp_name object for Sroutine_hash_entry.

  @note The lifetime of this object is bound to the lifetime of the
        Sroutine_hash_entry object.
        This should be fine as sp_name objects created by this constructor
        are mainly used for SP-cache lookups.

  @note Stored routine names are case insensitive. So for the proper key
        comparison, routine name is converted to the lower case while
        creating Sroutine_hash_entry. Hence the instance of sp_name created
        from it has the routine name in lower case.
        Since instances created by this constructor are mainly used for
        SP-cache lookups, routine name in lower case should work fine.

  @param rt          Sroutine_hash_entry with key containing database and
                     routine name.
  @param qname_buff  Buffer to be used for storing quoted routine name
                     (should be at least 2*NAME_LEN+1+1 bytes).
*/

sp_name::sp_name(const Sroutine_hash_entry *rt, char *qname_buff) {
  m_db.str = rt->db();
  m_db.length = rt->db_length();
  // Safe as sp_name is not changed in scenarios when this ctor is used.
  m_name.str = const_cast<char *>(rt->name());
  m_name.length = rt->name_length();
  m_qname.str = qname_buff;
  if (m_db.length) {
    strxmov(qname_buff, m_db.str, ".", m_name.str, NullS);
    m_qname.length = m_db.length + 1 + m_name.length;
  } else {
    my_stpcpy(qname_buff, m_name.str);
    m_qname.length = m_name.length;
  }
  m_explicit_name = false;
}

/**
  Init the qualified name from the db and name.
*/
void sp_name::init_qname(THD *thd) {
  const uint dot = !!m_db.length;
  /* m_qname format: [database + dot] + name + '\0' */
  m_qname.length = m_db.length + dot + m_name.length;
  if (!(m_qname.str = (char *)thd->alloc(m_qname.length + 1))) return;
  sprintf(m_qname.str, "%.*s%.*s%.*s", (int)m_db.length,
          (m_db.length ? m_db.str : ""), dot, ".", (int)m_name.length,
          m_name.str);
}

///////////////////////////////////////////////////////////////////////////
// sp_head implementation.
///////////////////////////////////////////////////////////////////////////

void sp_head::destroy(sp_head *sp) {
  if (!sp) return;

  /* Pull out main_mem_root as free_root will free the sp */
  MEM_ROOT own_root = std::move(sp->main_mem_root);

  sp->~sp_head();

  free_root(&own_root, MYF(0));
}

sp_head::sp_head(MEM_ROOT &&mem_root, enum_sp_type type)
    : m_type(type),
      m_flags(0),
      m_chistics(nullptr),
      m_sql_mode(0),
      m_explicit_name(false),
      m_created(0),
      m_modified(0),
      m_recursion_level(0),
      m_next_cached_sp(nullptr),
      m_first_instance(nullptr),
      m_first_free_instance(nullptr),
      m_last_cached_sp(nullptr),
      m_sroutines(key_memory_sp_head_main_root),
      m_trg_list(nullptr),
      main_mem_root(std::move(mem_root)),
      m_root_parsing_ctx(nullptr),
      m_instructions(&main_mem_root),
      m_sptabs(system_charset_info, key_memory_sp_head_main_root),
      m_sp_cache_version(0),
      m_creation_ctx(nullptr),
      unsafe_flags(0) {
  m_first_instance = this;
  m_first_free_instance = this;
  m_last_cached_sp = this;

  m_instructions.reserve(32);

  m_return_field_def.charset = nullptr;

  /*
    FIXME: the only use case when name is NULL is events, and it should
    be rewritten soon. Remove the else part and replace 'if' with
    an assert when this is done.
  */

  m_db = NULL_STR;
  m_name = NULL_STR;
  m_qname = NULL_STR;

  m_params = NULL_STR;

  m_defstr = NULL_STR;
  m_body = NULL_CSTR;
  m_body_utf8 = NULL_CSTR;

  m_trg_chistics.ordering_clause = TRG_ORDER_NONE;
  m_trg_chistics.anchor_trigger_name = NULL_CSTR;
}

void sp_head::init_sp_name(THD *thd, sp_name *spname) {
  /* Must be initialized in the parser. */

  DBUG_ASSERT(spname && spname->m_db.str && spname->m_db.length);

  /* We have to copy strings to get them into the right memroot. */

  m_db.length = spname->m_db.length;
  m_db.str = strmake_root(thd->mem_root, spname->m_db.str, spname->m_db.length);

  m_name.length = spname->m_name.length;
  m_name.str =
      strmake_root(thd->mem_root, spname->m_name.str, spname->m_name.length);

  m_explicit_name = spname->m_explicit_name;

  if (spname->m_qname.length == 0) spname->init_qname(thd);

  m_qname.length = spname->m_qname.length;
  m_qname.str = (char *)memdup_root(thd->mem_root, spname->m_qname.str,
                                    spname->m_qname.length + 1);
}

void sp_head::set_body_start(THD *thd, const char *begin_ptr) {
  m_parser_data.set_body_start_ptr(begin_ptr);

  thd->m_parser_state->m_lip.body_utf8_start(thd, begin_ptr);
}

void sp_head::set_body_end(THD *thd) {
  Lex_input_stream *lip = &thd->m_parser_state->m_lip; /* shortcut */
  const char *end_ptr = lip->get_cpp_ptr();            /* shortcut */

  /* Make the string of parameters. */

  {
    const char *p_start = m_parser_data.get_parameter_start_ptr();
    const char *p_end = m_parser_data.get_parameter_end_ptr();

    if (p_start && p_end) {
      m_params.length = p_end - p_start;
      m_params.str = thd->strmake(p_start, m_params.length);
    }
  }

  /* Remember end pointer for further dumping of whole statement. */

  thd->lex->stmt_definition_end = end_ptr;

  /* Make the string of body (in the original character set). */

  LEX_STRING body;
  body.length = end_ptr - m_parser_data.get_body_start_ptr();
  body.str = thd->strmake(m_parser_data.get_body_start_ptr(), body.length);
  trim_whitespace(thd->charset(), &body);
  m_body = to_lex_cstring(body);

  /* Make the string of UTF-body. */

  lip->body_utf8_append(end_ptr);

  LEX_STRING body_utf8;
  body_utf8.length = lip->get_body_utf8_length();
  body_utf8.str = thd->strmake(lip->get_body_utf8_str(), body_utf8.length);
  trim_whitespace(thd->charset(), &body_utf8);
  m_body_utf8 = to_lex_cstring(body_utf8);

  /*
    Make the string of whole stored-program-definition query (in the
    original character set).
  */

  m_defstr.length = end_ptr - lip->get_cpp_buf();
  m_defstr.str = thd->strmake(lip->get_cpp_buf(), m_defstr.length);
  trim_whitespace(thd->charset(), &m_defstr);
}

bool sp_head::setup_trigger_fields(THD *thd, Table_trigger_field_support *tfs,
                                   GRANT_INFO *subject_table_grant,
                                   bool need_fix_fields) {
  for (SQL_I_List<Item_trigger_field> *trig_field_list =
           m_list_of_trig_fields_item_lists.first;
       trig_field_list;
       trig_field_list = trig_field_list->first->next_trig_field_list) {
    for (Item_trigger_field *f = trig_field_list->first; f;
         f = f->next_trg_field) {
      f->setup_field(tfs, subject_table_grant);

      if (need_fix_fields && !f->fixed &&
          f->fix_fields(thd, (Item **)nullptr)) {
        return true;
      }
    }
  }

  return false;
}

void sp_head::mark_used_trigger_fields(TABLE *subject_table) {
  for (SQL_I_List<Item_trigger_field> *trig_field_list =
           m_list_of_trig_fields_item_lists.first;
       trig_field_list;
       trig_field_list = trig_field_list->first->next_trig_field_list) {
    for (Item_trigger_field *f = trig_field_list->first; f;
         f = f->next_trg_field) {
      if (f->field_idx == (uint)-1) {
        // We cannot mark fields which does not present in table.
        continue;
      }

      bitmap_set_bit(subject_table->read_set, f->field_idx);

      if (f->get_settable_routine_parameter())
        bitmap_set_bit(subject_table->write_set, f->field_idx);
    }
  }
}

/**
  Check whether any table's fields are used in trigger.

  @param [in] used_fields       bitmap of fields to check

  @return Check result
    @retval true   Some table fields are used in trigger
    @retval false  None of table fields are used in trigger
*/

bool sp_head::has_updated_trigger_fields(const MY_BITMAP *used_fields) const {
  for (SQL_I_List<Item_trigger_field> *trig_field_list =
           m_list_of_trig_fields_item_lists.first;
       trig_field_list;
       trig_field_list = trig_field_list->first->next_trig_field_list) {
    for (Item_trigger_field *f = trig_field_list->first; f;
         f = f->next_trg_field) {
      // We cannot check fields which does not present in table.
      if (f->field_idx != (uint)-1) {
        if (bitmap_is_set(used_fields, f->field_idx) &&
            f->get_settable_routine_parameter())
          return true;
      }
    }
  }

  return false;
}

sp_head::~sp_head() {
  LEX *lex;
  sp_instr *i;

  // Parsing of SP-body must have been already finished.
  DBUG_ASSERT(!m_parser_data.is_parsing_sp_body());

  for (uint ip = 0; (i = get_instr(ip)); ip++) ::destroy(i);

  ::destroy(m_root_parsing_ctx);

  /*
    If we have non-empty LEX stack then we just came out of parser with
    error. Now we should delete all auxiliary LEXes and restore original
    THD::lex. It is safe to not update LEX::ptr because further query
    string parsing and execution will be stopped anyway.
  */
  while ((lex = m_parser_data.pop_lex())) {
    THD *thd = lex->thd;
    thd->lex->sphead = nullptr;
    lex_end(thd->lex);
    delete thd->lex;
    thd->lex = lex;
  }

  sp_head::destroy(m_next_cached_sp);
}

Field *sp_head::create_result_field(THD *thd, size_t field_max_length,
                                    const char *field_name_or_null,
                                    TABLE *table) const {
  DBUG_ASSERT(!m_return_field_def.is_array);
  size_t field_length = !m_return_field_def.max_display_width_in_bytes()
                            ? field_max_length
                            : m_return_field_def.max_display_width_in_bytes();

  auto field_name =
      field_name_or_null != nullptr ? field_name_or_null : m_name.str;

  // Add 1 for null byte.
  table->record[0] =
      thd->mem_root->ArrayAlloc<uchar>(m_return_field_def.pack_length() + 1);
  if (table->record[0] == nullptr) return nullptr;

  DBUG_ASSERT(m_return_field_def.auto_flags == Field::NONE);
  Field *field =
      make_field(m_return_field_def, table->s, field_name, field_length,
                 table->record[0] + 1, table->record[0], 0);

  field->gcol_info = m_return_field_def.gcol_info;
  field->m_default_val_expr = m_return_field_def.m_default_val_expr;
  field->stored_in_db = m_return_field_def.stored_in_db;
  if (field) field->init(table);

  DBUG_ASSERT(field->pack_length() == m_return_field_def.pack_length());

  return field;
}

void sp_head::returns_type(THD *thd, String *result) const {
  DBUG_ASSERT(!m_return_field_def.is_array);
  DBUG_ASSERT(m_return_field_def.auto_flags == Field::NONE);

  TABLE table;
  TABLE_SHARE share;
  table.in_use = thd;
  table.s = &share;

  Field *field = make_field(m_return_field_def, &share, m_name.str,
                            m_return_field_def.max_display_width_in_bytes(),
                            nullptr, nullptr, 0);
  field->init(&table);  // Field::sql_type() uses Field::table::in_use
  field->sql_type(*result);

  if (field->has_charset()) {
    result->append(STRING_WITH_LEN(" CHARSET "));
    result->append(m_return_field_def.charset->csname);
    if (!(m_return_field_def.charset->state & MY_CS_PRIMARY)) {
      result->append(STRING_WITH_LEN(" COLLATE "));
      result->append(m_return_field_def.charset->name);
    }
  }

  ::destroy(field);
}

bool sp_head::execute(THD *thd, bool merge_da_on_success) {
  char saved_cur_db_name_buf[NAME_LEN + 1];
  LEX_STRING saved_cur_db_name = {saved_cur_db_name_buf,
                                  sizeof(saved_cur_db_name_buf)};
  bool cur_db_changed = false;
  bool err_status = false;
  uint ip = 0;
  sql_mode_t save_sql_mode;
  Query_arena *old_arena;
  /* per-instruction arena */
  MEM_ROOT execute_mem_root;
  Query_arena execute_arena(&execute_mem_root,
                            Query_arena::STMT_INITIALIZED_FOR_SP),
      backup_arena;
  query_id_t old_query_id;
  LEX *old_lex;
  Item_change_list old_change_list;
  String old_packet;
  Object_creation_ctx *saved_creation_ctx;
  Diagnostics_area *caller_da = thd->get_stmt_da();
  Diagnostics_area sp_da(false);

  /*
    Just reporting a stack overrun error
    (@sa check_stack_overrun()) requires stack memory for error
    message buffer. Thus, we have to put the below check
    relatively close to the beginning of the execution stack,
    where available stack margin is still big. As long as the check
    has to be fairly high up the call stack, the amount of memory
    we "book" for has to stay fairly high as well, and hence
    not very accurate. The number below has been calculated
    by trial and error, and reflects the amount of memory necessary
    to execute a single stored procedure instruction, be it either
    an SQL statement, or, heaviest of all, a CALL, which involves
    parsing and loading of another stored procedure into the cache
    (@sa db_load_routine() and Bug#10100).

    TODO: that should be replaced by proper handling of stack overrun error.

    Stack size depends on the platform:
      - for most platforms (8 * STACK_MIN_SIZE) is enough;
      - for Solaris SPARC 64 (10 * STACK_MIN_SIZE) is required.
      - for clang and UBSAN we need even more stack space.
  */

  {
#if defined(__sparc) && defined(__SUNPRO_CC)
    const int sp_stack_size = 10 * STACK_MIN_SIZE;
#elif defined(__clang__) && defined(HAVE_UBSAN)
    const int sp_stack_size = 16 * STACK_MIN_SIZE;
#else
    const int sp_stack_size = 8 * STACK_MIN_SIZE;
#endif

    if (check_stack_overrun(thd, sp_stack_size, (uchar *)&old_packet))
      return true;
  }

  opt_trace_disable_if_no_security_context_access(thd);

  /* init per-instruction memroot */
  init_sql_alloc(key_memory_sp_head_execute_root, &execute_mem_root,
                 MEM_ROOT_BLOCK_SIZE, 0);

  DBUG_ASSERT(!(m_flags & IS_INVOKED));
  m_flags |= IS_INVOKED;
  m_first_instance->m_first_free_instance = m_next_cached_sp;
  if (m_next_cached_sp) {
    DBUG_PRINT("info", ("first free for %p ++: %p->%p  level: %lu  flags %x",
                        m_first_instance, this, m_next_cached_sp,
                        m_next_cached_sp->m_recursion_level,
                        m_next_cached_sp->m_flags));
  }
  /*
    Check that if there are not any instances after this one then
    pointer to the last instance points on this instance or if there are
    some instances after this one then recursion level of next instance
    greater then recursion level of current instance on 1
  */
  DBUG_ASSERT((m_next_cached_sp == nullptr &&
               m_first_instance->m_last_cached_sp == this) ||
              (m_recursion_level + 1 == m_next_cached_sp->m_recursion_level));

  /*
    NOTE: The SQL Standard does not specify the context that should be
    preserved for stored routines. However, at SAP/Walldorf meeting it was
    decided that current database should be preserved.
  */
  if (m_db.length && (err_status = mysql_opt_change_db(
                          thd, to_lex_cstring(m_db), &saved_cur_db_name, false,
                          &cur_db_changed))) {
    goto done;
  }

  thd->is_slave_error = false;
  old_arena = thd->stmt_arena;

  /* Push a new Diagnostics Area. */
  thd->push_diagnostics_area(&sp_da);

  /*
    Switch query context. This has to be done early as this is sometimes
    allocated trough sql_alloc
  */
  saved_creation_ctx = m_creation_ctx->set_n_backup(thd);

  /*
    We have to save/restore this info when we are changing call level to
    be able properly do close_thread_tables() in instructions.
  */
  old_query_id = thd->query_id;
  save_sql_mode = thd->variables.sql_mode;
  thd->variables.sql_mode = m_sql_mode;
  /**
    When inside a substatement (a stored function or trigger
    statement), clear the metadata observer in THD, if any.
    Remember the value of the observer here, to be able
    to restore it when leaving the substatement.

    We reset the observer to suppress errors when a substatement
    uses temporary tables. If a temporary table does not exist
    at start of the main statement, it's not prelocked
    and thus is not validated with other prelocked tables.

    Later on, when the temporary table is opened, metadata
    versions mismatch, expectedly.

    The proper solution for the problem is to re-validate tables
    of substatements (Bug#12257, Bug#27011, Bug#32868, Bug#33000),
    but it's not implemented yet.
  */
  thd->push_reprepare_observer(nullptr);

  /*
    It is also more efficient to save/restore current thd->lex once when
    do it in each instruction
  */
  old_lex = thd->lex;
  /*
    We should also save Item tree change list to avoid rollback something
    too early in the calling query.
  */
  thd->change_list.move_elements_to(&old_change_list);

  if (thd->is_classic_protocol()) {
    /*
      Cursors will use thd->packet, so they may corrupt data which was
      prepared for sending by upper level. OTOH cursors in the same routine
      can share this buffer safely so let use use routine-local packet
      instead of having own packet buffer for each cursor.

      It is probably safe to use same thd->convert_buff everywhere.
    */
    old_packet.swap(*thd->get_protocol_classic()->get_output_packet());
  }

  /*
    Switch to per-instruction arena here. We can do it since we cleanup
    arena after every instruction.
  */
  thd->swap_query_arena(execute_arena, &backup_arena);

  /*
    Save callers arena in order to store instruction results and out
    parameters in it later during sp_eval_func_item()
  */
  thd->sp_runtime_ctx->callers_arena = &backup_arena;

#if defined(ENABLED_PROFILING)
  /* Discard the initial part of executing routines. */
  thd->profiling->discard_current_query();
#endif
  do {
    sp_instr *i;

#if defined(ENABLED_PROFILING)
    /*
     Treat each "instr" of a routine as discrete unit that could be profiled.
     Profiling only records information for segments of code that set the
     source of the query, and almost all kinds of instructions in s-p do not.
    */
    thd->profiling->finish_current_query();
    thd->profiling->start_new_query("continuing inside routine");
#endif

    /* get_instr returns NULL when we're done. */
    i = get_instr(ip);
    if (i == nullptr) {
#if defined(ENABLED_PROFILING)
      thd->profiling->discard_current_query();
#endif
      break;
    }

    DBUG_PRINT("execute", ("Instruction %u", ip));

    /*
      We need to reset start_time to allow for time to flow inside a stored
      procedure. This is only done for SP since time is suppose to be constant
      during execution of triggers and functions.
    */
    reset_start_time_for_sp(thd);

    /*
      We have to set thd->stmt_arena before executing the instruction
      to store in the instruction item list all new items, created
      during the first execution (for example expanding of '*' or the
      items made during other permanent subquery transformations).
    */
    thd->stmt_arena = &i->m_arena;

    /*
      Will write this SP statement into binlog separately.
      TODO: consider changing the condition to "not inside event union".
    */
    if (thd->locked_tables_mode <= LTM_LOCK_TABLES)
      thd->user_var_events_alloc = thd->mem_root;

    sql_digest_state digest_state;
    sql_digest_state *parent_digest = thd->m_digest;
    thd->m_digest = &digest_state;

#ifdef HAVE_PSI_STATEMENT_INTERFACE
    PSI_statement_locker_state psi_state;
    PSI_statement_info *psi_info = i->get_psi_info();
    PSI_statement_locker *parent_locker;

    parent_locker = thd->m_statement_psi;
    thd->m_statement_psi = MYSQL_START_STATEMENT(
        &psi_state, psi_info->m_key, thd->db().str, thd->db().length,
        thd->charset(), this->m_sp_share);
#endif

    /*
      For now, we're mostly concerned with sp_instr_stmt, but that's
      likely to change in the future, so we'll do it right from the
      start.
    */
    if (thd->rewritten_query().length()) thd->reset_rewritten_query();

    err_status = i->execute(thd, &ip);

#ifdef HAVE_PSI_STATEMENT_INTERFACE
    MYSQL_END_STATEMENT(thd->m_statement_psi, thd->get_stmt_da());
    thd->m_statement_psi = parent_locker;
#endif

    thd->m_digest = parent_digest;

    cleanup_items(i->m_arena.item_list());

    /*
      If we've set thd->user_var_events_alloc to mem_root of this SP
      statement, clean all the events allocated in it.
    */
    if (thd->locked_tables_mode <= LTM_LOCK_TABLES) {
      thd->user_var_events.clear();
      thd->user_var_events_alloc = nullptr;  // DEBUG
    }

    /* we should cleanup free_list and memroot, used by instruction */

    thd->cleanup_after_query();
    free_root(&execute_mem_root, MYF(0));

    /*
      Find and process SQL handlers unless it is a fatal error (fatal
      errors are not catchable by SQL handlers) or the connection has been
      killed during execution.
    */
#ifdef HAVE_PSI_ERROR_INTERFACE
    uint error_num = 0;
    if (thd->is_error()) error_num = thd->get_stmt_da()->mysql_errno();
#endif
    if (!thd->is_fatal_error() && !thd->killed &&
        thd->sp_runtime_ctx->handle_sql_condition(thd, &ip, i)) {
      err_status = false;
#ifdef HAVE_PSI_ERROR_INTERFACE
      if (error_num) MYSQL_LOG_ERROR(error_num, PSI_ERROR_OPERATION_HANDLED);
#endif
    }

    /* Reset sp_rcontext::end_partial_result_set flag. */
    thd->sp_runtime_ctx->end_partial_result_set = false;

  } while (!err_status && !thd->killed && !thd->is_fatal_error());

#if defined(ENABLED_PROFILING)
  thd->profiling->finish_current_query();
  thd->profiling->start_new_query("tail end of routine");
#endif

  /* Restore query context. */

  m_creation_ctx->restore_env(thd, saved_creation_ctx);

  /* Restore arena. */

  thd->swap_query_arena(backup_arena, &execute_arena);

  thd->sp_runtime_ctx
      ->pop_all_cursors();  // To avoid memory leaks after an error

  if (thd->is_classic_protocol()) /* Restore all saved */
    old_packet.swap(*thd->get_protocol_classic()->get_output_packet());
  DBUG_ASSERT(thd->change_list.is_empty());
  old_change_list.move_elements_to(&thd->change_list);
  thd->lex = old_lex;
  thd->set_query_id(old_query_id);
  thd->variables.sql_mode = save_sql_mode;
  thd->pop_reprepare_observer();

  thd->stmt_arena = old_arena;

  if (err_status && thd->is_error() && !caller_da->is_error()) {
    /*
      If the SP ended with an exception, transfer the exception condition
      information to the Diagnostics Area of the caller.

      Note that no error might be set yet in the case of kill.
      It will be set later by mysql_execute_command() / execute_trigger().

      In the case of multi update, it is possible that we can end up
      executing a trigger after the update has failed. In this case,
      keep the exception condition from the caller_da and don't transfer.
    */
    caller_da->set_error_status(thd->get_stmt_da()->mysql_errno(),
                                thd->get_stmt_da()->message_text(),
                                thd->get_stmt_da()->returned_sqlstate());
  }

  /*
    - conditions generated during trigger execution should not be
    propagated to the caller on success;   (merge_da_on_success)
    - if there was an exception during execution, conditions should be
    propagated to the caller in any case.  (err_status)
  */
  if (err_status || merge_da_on_success) {
    /*
      If a routine body is empty or if a routine did not generate any
      conditions, do not duplicate our own contents by appending the contents
      of the called routine. We know that the called routine did not change its
      Diagnostics Area.

      On the other hand, if the routine body is not empty and some statement
      in the routine generates a condition, Diagnostics Area is guaranteed to
      have changed. In this case we know that the routine Diagnostics Area
      contains only new conditions, and thus we perform a copy.

      We don't use push_warning() here as to avoid invocation of
      condition handlers or escalation of warnings to errors.
    */
    if (!err_status && thd->get_stmt_da() != &sp_da) {
      /*
        If we are RETURNing directly from a handler and the handler has
        executed successfully, only transfer the conditions that were
        raised during handler execution. Conditions that were present
        when the handler was activated, are considered handled.
      */
      caller_da->copy_new_sql_conditions(thd, thd->get_stmt_da());
    } else  // err_status || thd->get_stmt_da() == sp_da
    {
      /*
        If we ended with an exception, or the SP exited without any handler
        active, transfer all conditions to the Diagnostics Area of the caller.
      */
      caller_da->copy_sql_conditions_from_da(thd, thd->get_stmt_da());
    }
  }

  // Restore the caller's original Diagnostics Area.
  while (thd->get_stmt_da() != &sp_da) thd->pop_diagnostics_area();
  thd->pop_diagnostics_area();
  DBUG_ASSERT(thd->get_stmt_da() == caller_da);

done:
  DBUG_PRINT(
      "info",
      ("err_status: %d  killed: %d  is_slave_error: %d  report_error: %d",
       err_status, thd->killed.load(), thd->is_slave_error, thd->is_error()));

  if (thd->killed) err_status = true;
  /*
    If the DB has changed, the pointer has changed too, but the
    original thd->db will then have been freed
  */
  if (cur_db_changed && thd->killed != THD::KILL_CONNECTION) {
    /*
      Force switching back to the saved current database, because it may be
      NULL. In this case, mysql_change_db() would generate an error.
    */

    err_status |= mysql_change_db(thd, to_lex_cstring(saved_cur_db_name), true);
  }
  m_flags &= ~IS_INVOKED;
  DBUG_PRINT("info", ("first free for %p --: %p->%p, level: %lu, flags %x",
                      m_first_instance, m_first_instance->m_first_free_instance,
                      this, m_recursion_level, m_flags));
  /*
    Check that we have one of following:

    1) there are not free instances which means that this instance is last
    in the list of instances (pointer to the last instance point on it and
    there are not other instances after this one in the list)

    2) There are some free instances which mean that first free instance
    should go just after this one and recursion level of that free instance
    should be on 1 more then recursion level of this instance.
  */
  DBUG_ASSERT((m_first_instance->m_first_free_instance == nullptr &&
               this == m_first_instance->m_last_cached_sp &&
               m_next_cached_sp == nullptr) ||
              (m_first_instance->m_first_free_instance != nullptr &&
               m_first_instance->m_first_free_instance == m_next_cached_sp &&
               m_first_instance->m_first_free_instance->m_recursion_level ==
                   m_recursion_level + 1));
  m_first_instance->m_first_free_instance = this;

  return err_status;
}

bool sp_head::execute_trigger(THD *thd, const LEX_CSTRING &db_name,
                              const LEX_CSTRING &table_name,
                              GRANT_INFO *grant_info) {
  sp_rcontext *parent_sp_runtime_ctx = thd->sp_runtime_ctx;
  bool err_status = false;
  MEM_ROOT call_mem_root;
  Query_arena call_arena(&call_mem_root, Query_arena::STMT_INITIALIZED_FOR_SP);
  Query_arena backup_arena;

  DBUG_TRACE;
  DBUG_PRINT("info", ("trigger %s", m_name.str));

  Security_context *save_ctx = nullptr;
  LEX_CSTRING definer_user = {m_definer_user.str, m_definer_user.length};
  LEX_CSTRING definer_host = {m_definer_host.str, m_definer_host.length};

  /*
    While parsing CREATE TRIGGER statement or loading trigger metadata from
    the Data Dictionary we guarantee that definer hasn't empty value.
    It means, that trigger can't never be NOT-SUID.
  */
  DBUG_ASSERT(m_chistics->suid != SP_IS_NOT_SUID);
  if (m_security_ctx.change_security_context(thd, definer_user, definer_host,
                                             m_db.str, &save_ctx))
    return true;

  /*
    Fetch information about table-level privileges for subject table into
    GRANT_INFO instance. The access check itself will happen in
    Item_trigger_field, where this information will be used along with
    information about column-level privileges.
  */

  fill_effective_table_privileges(thd, grant_info, db_name.str, table_name.str);

  /* Check that the definer has TRIGGER privilege on the subject table. */

  if (!(grant_info->privilege & TRIGGER_ACL)) {
    char priv_desc[128];
    get_privilege_desc(priv_desc, sizeof(priv_desc), TRIGGER_ACL);

    my_error(ER_TABLEACCESS_DENIED_ERROR, MYF(0), priv_desc,
             thd->security_context()->priv_user().str,
             thd->security_context()->host_or_ip().str, table_name.str);

    m_security_ctx.restore_security_context(thd, save_ctx);
    return true;
  }
  /*
    Optimizer trace note: we needn't explicitly test here that the connected
    user has TRIGGER privilege: assume he doesn't have it; two possibilities:
    - connected user == definer: then we threw an error just above;
    - connected user != definer: then in sp_head::execute(), when checking the
    security context we will disable tracing.
  */

  /*
    Prepare arena and memroot for objects which lifetime is whole
    duration of trigger call (sp_rcontext, it's tables and items,
    sp_cursor and Item_cache holders for case expressions).  We can't
    use caller's arena/memroot for those objects because in this case
    some fixed amount of memory will be consumed for each trigger
    invocation and so statements which involve lot of them will hog
    memory.

    TODO: we should create sp_rcontext once per command and reuse it
    on subsequent executions of a trigger.
  */
  init_sql_alloc(key_memory_sp_head_call_root, &call_mem_root,
                 MEM_ROOT_BLOCK_SIZE, 0);
  thd->swap_query_arena(call_arena, &backup_arena);

  sp_rcontext *trigger_runtime_ctx =
      sp_rcontext::create(thd, m_root_parsing_ctx, nullptr);

  if (!trigger_runtime_ctx) {
    err_status = true;
    goto err_with_cleanup;
  }

  trigger_runtime_ctx->sp = this;
  thd->sp_runtime_ctx = trigger_runtime_ctx;

#ifdef HAVE_PSI_SP_INTERFACE
  PSI_sp_locker_state psi_state;
  PSI_sp_locker *locker;

  locker = MYSQL_START_SP(&psi_state, m_sp_share);
#endif
  err_status = execute(thd, false);
#ifdef HAVE_PSI_SP_INTERFACE
  MYSQL_END_SP(locker);
#endif

err_with_cleanup:
  thd->swap_query_arena(backup_arena, &call_arena);

  m_security_ctx.restore_security_context(thd, save_ctx);

  ::destroy(trigger_runtime_ctx);
  call_arena.free_items();
  free_root(&call_mem_root, MYF(0));
  thd->sp_runtime_ctx = parent_sp_runtime_ctx;

  if (thd->killed) thd->send_kill_message();

  return err_status;
}

bool sp_head::execute_function(THD *thd, Item **argp, uint argcount,
                               Field *return_value_fld) {
  ulonglong binlog_save_options = 0;
  bool need_binlog_call = false;
  uint arg_no;
  sp_rcontext *parent_sp_runtime_ctx = thd->sp_runtime_ctx;
  char buf[STRING_BUFFER_USUAL_SIZE];
  String binlog_buf(buf, sizeof(buf), &my_charset_bin);
  bool err_status = false;
  MEM_ROOT call_mem_root;
  Query_arena call_arena(&call_mem_root, Query_arena::STMT_INITIALIZED_FOR_SP);
  Query_arena backup_arena;

  DBUG_TRACE;
  DBUG_PRINT("info", ("function %s", m_name.str));

  // Resetting THD::where to its default value
  thd->where = THD::DEFAULT_WHERE;
  /*
    Check that the function is called with all specified arguments.

    If it is not, use my_error() to report an error, or it will not terminate
    the invoking query properly.
  */
  if (argcount != m_root_parsing_ctx->context_var_count()) {
    /*
      Need to use my_error here, or it will not terminate the
      invoking query properly.
    */
    my_error(ER_SP_WRONG_NO_OF_ARGS, MYF(0), "FUNCTION", m_qname.str,
             m_root_parsing_ctx->context_var_count(), argcount);
    return true;
  }

  /*
    Prepare arena and memroot for objects which lifetime is whole
    duration of function call (sp_rcontext, it's tables and items,
    sp_cursor and Item_cache holders for case expressions).
    We can't use caller's arena/memroot for those objects because
    in this case some fixed amount of memory will be consumed for
    each function/trigger invocation and so statements which involve
    lot of them will hog memory.
    TODO: we should create sp_rcontext once per command and reuse
    it on subsequent executions of a function/trigger.
  */
  init_sql_alloc(key_memory_sp_head_call_root, &call_mem_root,
                 MEM_ROOT_BLOCK_SIZE, 0);
  thd->swap_query_arena(call_arena, &backup_arena);

  sp_rcontext *func_runtime_ctx =
      sp_rcontext::create(thd, m_root_parsing_ctx, return_value_fld);

  if (!func_runtime_ctx) {
    thd->swap_query_arena(backup_arena, &call_arena);
    err_status = true;
    goto err_with_cleanup;
  }

  func_runtime_ctx->sp = this;

  /*
    We have to switch temporarily back to callers arena/memroot.
    Function arguments belong to the caller and so the may reference
    memory which they will allocate during calculation long after
    this function call will be finished (e.g. in Item::cleanup()).
  */
  thd->swap_query_arena(backup_arena, &call_arena);

  /*
    Pass arguments.

    Note, THD::sp_runtime_ctx must not be switched before the arguments are
    passed. Values are taken from the caller's runtime context and set to the
    runtime context of this function.
  */
  for (arg_no = 0; arg_no < argcount; arg_no++) {
    /* Arguments must be fixed in Item_func_sp::fix_fields */
    DBUG_ASSERT(argp[arg_no]->fixed);

    err_status = func_runtime_ctx->set_variable(thd, arg_no, &(argp[arg_no]));

    if (err_status) goto err_with_cleanup;
  }

  /*
    If row-based binlogging, we don't need to binlog the function's call, let
    each substatement be binlogged its way.
  */
  need_binlog_call = mysql_bin_log.is_open() &&
                     (thd->variables.option_bits & OPTION_BIN_LOG) &&
                     !thd->is_current_stmt_binlog_format_row();

  /*
    Remember the original arguments for unrolled replication of functions
    before they are changed by execution.

    Note, THD::sp_runtime_ctx must not be switched before the arguments are
    logged. Values are taken from the caller's runtime context.
  */
  if (need_binlog_call) {
    binlog_buf.length(0);
    binlog_buf.append(STRING_WITH_LEN("SELECT "));
    append_identifier(thd, &binlog_buf, m_db.str, m_db.length);
    binlog_buf.append('.');
    append_identifier(thd, &binlog_buf, m_name.str, m_name.length);
    binlog_buf.append('(');
    for (arg_no = 0; arg_no < argcount; arg_no++) {
      String str_value_holder;
      String *str_value;

      if (arg_no) binlog_buf.append(',');

      str_value = sp_get_item_value(thd, func_runtime_ctx->get_item(arg_no),
                                    &str_value_holder);

      if (str_value)
        binlog_buf.append(*str_value);
      else
        binlog_buf.append(STRING_WITH_LEN("NULL"));
    }
    binlog_buf.append(')');
  }

  thd->sp_runtime_ctx = func_runtime_ctx;

  Security_context *save_security_ctx;
  if (set_security_ctx(thd, &save_security_ctx)) {
    err_status = true;
    goto err_with_cleanup;
  }

  if (need_binlog_call) {
    query_id_t q;
    thd->user_var_events.clear();
    /*
      In case of artificially constructed events for function calls
      we have separate union for each such event and hence can't use
      query_id of real calling statement as the start of all these
      unions (this will break logic of replication of user-defined
      variables). So we use artificial value which is guaranteed to
      be greater than all query_id's of all statements belonging
      to previous events/unions.
      Possible alternative to this is logging of all function invocations
      as one select and not resetting THD::user_var_events before
      each invocation.
    */
    q = atomic_global_query_id;
    mysql_bin_log.start_union_events(thd, q + 1);
    binlog_save_options = thd->variables.option_bits;
    thd->variables.option_bits &= ~OPTION_BIN_LOG;
  }

  opt_trace_disable_if_no_stored_proc_func_access(thd, this);

  /*
    Switch to call arena/mem_root so objects like sp_cursor or
    Item_cache holders for case expressions can be allocated on it.

    TODO: In future we should associate call arena/mem_root with
          sp_rcontext and allocate all these objects (and sp_rcontext
          itself) on it directly rather than juggle with arenas.
  */
  thd->swap_query_arena(call_arena, &backup_arena);

#ifdef HAVE_PSI_SP_INTERFACE
  PSI_sp_locker_state psi_state;
  PSI_sp_locker *locker;

  locker = MYSQL_START_SP(&psi_state, m_sp_share);
#endif
  err_status = execute(thd, true);
#ifdef HAVE_PSI_SP_INTERFACE
  MYSQL_END_SP(locker);
#endif

  thd->swap_query_arena(backup_arena, &call_arena);

  if (need_binlog_call) {
    mysql_bin_log.stop_union_events(thd);
    thd->variables.option_bits = binlog_save_options;
    if (thd->binlog_evt_union.unioned_events) {
      int errcode = query_error_code(thd, thd->killed == THD::NOT_KILLED);
      Query_log_event qinfo(thd, binlog_buf.ptr(), binlog_buf.length(),
                            thd->binlog_evt_union.unioned_events_trans, false,
                            false, errcode);
      if (mysql_bin_log.write_event(&qinfo) &&
          thd->binlog_evt_union.unioned_events_trans) {
        push_warning(thd, Sql_condition::SL_WARNING, ER_UNKNOWN_ERROR,
                     "Invoked ROUTINE modified a transactional table but MySQL "
                     "failed to reflect this change in the binary log");
        err_status = true;
      }
      thd->user_var_events.clear();
      /* Forget those values, in case more function calls are binlogged: */
      thd->stmt_depends_on_first_successful_insert_id_in_prev_stmt = false;
      thd->auto_inc_intervals_in_cur_stmt_for_binlog.empty();
    }
  }

  if (!err_status) {
    /* We need result only in function but not in trigger */

    if (!thd->sp_runtime_ctx->is_return_value_set()) {
      my_error(ER_SP_NORETURNEND, MYF(0), m_name.str);
      err_status = true;
    }
  }

  m_security_ctx.restore_security_context(thd, save_security_ctx);

err_with_cleanup:
  ::destroy(func_runtime_ctx);
  call_arena.free_items();
  free_root(&call_mem_root, MYF(0));
  thd->sp_runtime_ctx = parent_sp_runtime_ctx;

  /*
    If not inside a procedure and a function printing warning
    messages.
  */
  if (need_binlog_call && thd->sp_runtime_ctx == nullptr &&
      !thd->binlog_evt_union.do_union)
    thd->issue_unsafe_warnings();

  return err_status;
}

bool sp_head::execute_procedure(THD *thd, List<Item> *args) {
  bool err_status = false;
  uint params = m_root_parsing_ctx->context_var_count();
  /* Query start time may be reset in a multi-stmt SP; keep this for later. */
  ulonglong utime_before_sp_exec = thd->utime_after_lock;
  sp_rcontext *parent_sp_runtime_ctx = thd->sp_runtime_ctx;
  sp_rcontext *sp_runtime_ctx_saved = thd->sp_runtime_ctx;
  bool save_enable_slow_log = false;
  bool save_log_general = false;

  DBUG_TRACE;
  DBUG_PRINT("info", ("procedure %s", m_name.str));

  // Argument count has been validated in prepare function.
  DBUG_ASSERT((args != nullptr ? args->elements : 0) == params);

  if (!parent_sp_runtime_ctx) {
    // Create a temporary old context. We need it to pass OUT-parameter values.
    parent_sp_runtime_ctx =
        sp_rcontext::create(thd, m_root_parsing_ctx, nullptr);

    if (!parent_sp_runtime_ctx) return true;

    parent_sp_runtime_ctx->sp = nullptr;
    thd->sp_runtime_ctx = parent_sp_runtime_ctx;

    /* set callers_arena to thd, for upper-level function to work */
    thd->sp_runtime_ctx->callers_arena = thd;
  }

  sp_rcontext *proc_runtime_ctx =
      sp_rcontext::create(thd, m_root_parsing_ctx, nullptr);

  if (!proc_runtime_ctx) {
    thd->sp_runtime_ctx = sp_runtime_ctx_saved;

    if (!sp_runtime_ctx_saved) ::destroy(parent_sp_runtime_ctx);

    return true;
  }

  proc_runtime_ctx->sp = this;

  if (params > 0) {
    List_iterator<Item> it_args(*args);

    DBUG_PRINT("info", (" %.*s: eval args", (int)m_name.length, m_name.str));

    for (uint i = 0; i < params; i++) {
      Item *arg_item = it_args++;

      if (!arg_item) break;

      sp_variable *spvar = m_root_parsing_ctx->find_variable(i);

      if (!spvar) continue;

      if (spvar->mode != sp_variable::MODE_IN) {
        Settable_routine_parameter *srp =
            arg_item->get_settable_routine_parameter();

        if (!srp) {
          my_error(ER_SP_NOT_VAR_ARG, MYF(0), i + 1, m_qname.str);
          err_status = true;
          break;
        }
      }

      if (spvar->mode == sp_variable::MODE_OUT) {
        Item_null *null_item = new Item_null();

        if (!null_item ||
            proc_runtime_ctx->set_variable(thd, i, (Item **)&null_item)) {
          err_status = true;
          break;
        }
      } else {
        if (proc_runtime_ctx->set_variable(thd, i, it_args.ref())) {
          err_status = true;
          break;
        }
      }

      if (thd->variables.session_track_transaction_info > TX_TRACK_NONE) {
        TX_TRACKER_GET(tst);
        tst->add_trx_state_from_thd(thd);
      }
    }

    /*
      Okay, got values for all arguments. Close tables that might be used by
      arguments evaluation. If arguments evaluation required prelocking mode,
      we'll leave it here.
    */
    thd->lex->unit->cleanup(thd, true);

    if (!thd->in_sub_stmt) {
      thd->get_stmt_da()->set_overwrite_status(true);
      thd->is_error() ? trans_rollback_stmt(thd) : trans_commit_stmt(thd);
      thd->get_stmt_da()->set_overwrite_status(false);
    }

    thd_proc_info(thd, "closing tables");
    close_thread_tables(thd);
    thd_proc_info(thd, nullptr);

    if (!thd->in_sub_stmt) {
      if (thd->transaction_rollback_request) {
        trans_rollback_implicit(thd);
        thd->mdl_context.release_transactional_locks();
      } else if (!thd->in_multi_stmt_transaction_mode())
        thd->mdl_context.release_transactional_locks();
      else
        thd->mdl_context.release_statement_locks();
    }

    thd->rollback_item_tree_changes();

    DBUG_PRINT("info",
               (" %.*s: eval args done", (int)m_name.length, m_name.str));
  }
  if (!(m_flags & LOG_SLOW_STATEMENTS) && thd->enable_slow_log) {
    DBUG_PRINT("info", ("Disabling slow log for the execution"));
    save_enable_slow_log = true;
    thd->enable_slow_log = false;
  }
  if (!(m_flags & LOG_GENERAL_LOG) &&
      !(thd->variables.option_bits & OPTION_LOG_OFF)) {
    DBUG_PRINT("info", ("Disabling general log for the execution"));
    save_log_general = true;
    /* disable this bit */
    thd->variables.option_bits |= OPTION_LOG_OFF;
  }
  thd->sp_runtime_ctx = proc_runtime_ctx;

  Security_context *save_security_ctx = nullptr;
  if (!err_status) err_status = set_security_ctx(thd, &save_security_ctx);

  opt_trace_disable_if_no_stored_proc_func_access(thd, this);

#ifdef HAVE_PSI_SP_INTERFACE
  PSI_sp_locker_state psi_state;
  PSI_sp_locker *locker;

  locker = MYSQL_START_SP(&psi_state, m_sp_share);
#endif
  if (!err_status) err_status = execute(thd, true);
#ifdef HAVE_PSI_SP_INTERFACE
  MYSQL_END_SP(locker);
#endif

  if (save_log_general) thd->variables.option_bits &= ~OPTION_LOG_OFF;
  if (save_enable_slow_log) thd->enable_slow_log = true;
  /*
    In the case when we weren't able to employ reuse mechanism for
    OUT/INOUT parameters, we should reallocate memory. This
    allocation should be done on the arena which will live through
    all execution of calling routine.
  */
  thd->sp_runtime_ctx->callers_arena = parent_sp_runtime_ctx->callers_arena;

  if (!err_status && params > 0) {
    List_iterator<Item> it_args(*args);

    /*
      Copy back all OUT or INOUT values to the previous frame, or
      set global user variables
    */
    for (uint i = 0; i < params; i++) {
      Item *arg_item = it_args++;

      if (!arg_item) break;

      sp_variable *spvar = m_root_parsing_ctx->find_variable(i);

      if (spvar->mode == sp_variable::MODE_IN) continue;

      Settable_routine_parameter *srp =
          arg_item->get_settable_routine_parameter();

      DBUG_ASSERT(srp);

      if (srp->set_value(thd, parent_sp_runtime_ctx,
                         proc_runtime_ctx->get_item_addr(i))) {
        err_status = true;
        break;
      }

      Send_field *out_param_info = new (thd->mem_root) Send_field();
      proc_runtime_ctx->get_item(i)->make_field(out_param_info);
      out_param_info->db_name = m_db.str;
      out_param_info->table_name = m_name.str;
      out_param_info->org_table_name = m_name.str;
      out_param_info->col_name = spvar->name.str;
      out_param_info->org_col_name = spvar->name.str;

      srp->set_out_param_info(out_param_info);
    }
  }

  if (save_security_ctx)
    m_security_ctx.restore_security_context(thd, save_security_ctx);

  if (!sp_runtime_ctx_saved) ::destroy(parent_sp_runtime_ctx);

  ::destroy(proc_runtime_ctx);
  thd->sp_runtime_ctx = sp_runtime_ctx_saved;
  thd->utime_after_lock = utime_before_sp_exec;

  /*
    If not insided a procedure and a function printing warning
    messages.
  */
  bool need_binlog_call = mysql_bin_log.is_open() &&
                          (thd->variables.option_bits & OPTION_BIN_LOG) &&
                          !thd->is_current_stmt_binlog_format_row();
  if (need_binlog_call && thd->sp_runtime_ctx == nullptr &&
      !thd->binlog_evt_union.do_union)
    thd->issue_unsafe_warnings();

  return err_status;
}

bool sp_head::reset_lex(THD *thd) {
  LEX *oldlex = thd->lex;

  LEX *sublex = new (thd->mem_root) st_lex_local;

  if (!sublex) return true;

  thd->lex = sublex;
  m_parser_data.push_lex(oldlex);

  /* Reset most stuff. */
  lex_start(thd);

  /* And keep the SP stuff too */
  sublex->sphead = oldlex->sphead;
  sublex->set_sp_current_parsing_ctx(oldlex->get_sp_current_parsing_ctx());
  sublex->sp_lex_in_use = false;

  /* Reset part of parser state which needs this. */
  thd->m_parser_state->m_yacc.reset_before_substatement();

  return false;
}

bool sp_head::restore_lex(THD *thd) {
  LEX *sublex = thd->lex;

  sublex->set_trg_event_type_for_tables();

  LEX *oldlex = m_parser_data.pop_lex();

  if (!oldlex) return false;  // Nothing to restore

  /* If this substatement is unsafe, the entire routine is too. */
  DBUG_PRINT("info", ("lex->get_stmt_unsafe_flags: 0x%x",
                      thd->lex->get_stmt_unsafe_flags()));
  unsafe_flags |= sublex->get_stmt_unsafe_flags();

  /*
    Add routines which are used by statement to respective set for
    this routine.
  */
  if (sublex->sroutines != nullptr)
    sp_update_sp_used_routines(&m_sroutines, *sublex->sroutines);

  /* If this substatement is a update query, then mark MODIFIES_DATA */
  if (is_update_query(sublex->sql_command)) m_flags |= MODIFIES_DATA;

  /*
    Merge tables used by this statement (but not by its functions or
    procedures) to multiset of tables used by this routine.
  */
  merge_table_list(thd, sublex->query_tables, sublex);

  /* Update m_sptabs_sorted to be in sync with m_sptabs. */
  m_sptabs_sorted.clear();
  for (auto &key_and_value : m_sptabs) {
    m_sptabs_sorted.push_back(key_and_value.second);
  }
  std::sort(m_sptabs_sorted.begin(), m_sptabs_sorted.end(),
            [](const SP_TABLE *a, const SP_TABLE *b) {
              return to_string(a->qname) < to_string(b->qname);
            });

  if (!sublex->sp_lex_in_use) {
    sublex->sphead = nullptr;
    lex_end(sublex);
    delete sublex;
  }

  thd->lex = oldlex;
  return false;
}

void sp_head::set_info(longlong created, longlong modified,
                       st_sp_chistics *chistics, sql_mode_t sql_mode) {
  m_created = created;
  m_modified = modified;
  m_chistics = (st_sp_chistics *)memdup_root(&main_mem_root, (char *)chistics,
                                             sizeof(*chistics));
  if (m_chistics->comment.length == 0)
    m_chistics->comment.str = nullptr;
  else
    m_chistics->comment.str = strmake_root(
        &main_mem_root, m_chistics->comment.str, m_chistics->comment.length);
  m_sql_mode = sql_mode;
}

void sp_head::set_definer(const char *definer, size_t definerlen) {
  char user_name_holder[USERNAME_LENGTH + 1];
  LEX_CSTRING user_name = {user_name_holder, USERNAME_LENGTH};

  char host_name_holder[HOSTNAME_LENGTH + 1];
  LEX_CSTRING host_name = {host_name_holder, HOSTNAME_LENGTH};

  parse_user(definer, definerlen, user_name_holder, &user_name.length,
             host_name_holder, &host_name.length);

  set_definer(user_name, host_name);
}

void sp_head::set_definer(const LEX_CSTRING &user_name,
                          const LEX_CSTRING &host_name) {
  m_definer_user.str =
      strmake_root(&main_mem_root, user_name.str, user_name.length);
  m_definer_user.length = user_name.length;

  m_definer_host.str =
      strmake_root(&main_mem_root, host_name.str, host_name.length);
  m_definer_host.length = host_name.length;
}

bool sp_head::add_instr(THD *thd, sp_instr *instr) {
  m_parser_data.process_new_sp_instr(thd, instr);

  if (m_type == enum_sp_type::TRIGGER &&
      m_cur_instr_trig_field_items.elements) {
    SQL_I_List<Item_trigger_field> *instr_trig_fld_list;
    /*
      Move all the Item_trigger_field from "sp_head::
      m_cur_instr_trig_field_items" to the per instruction Item_trigger_field
      list "sp_lex_instr::m_trig_field_list" and clear "sp_head::
      m_cur_instr_trig_field_items".
    */
    if ((instr_trig_fld_list = instr->get_instr_trig_field_list()) != nullptr) {
      m_cur_instr_trig_field_items.save_and_clear(instr_trig_fld_list);
      m_list_of_trig_fields_item_lists.link_in_list(
          instr_trig_fld_list,
          &instr_trig_fld_list->first->next_trig_field_list);
    }
  }

  /*
    Memory root of every instruction is designated for permanent
    transformations (optimizations) made on the parsed tree during
    the first execution. It points to the memory root of the
    entire stored procedure, as their life span is equal.
  */
  instr->m_arena.mem_root = get_persistent_mem_root();

  return m_instructions.push_back(instr);
}

void sp_head::optimize() {
  List<sp_branch_instr> bp;
  sp_instr *i;
  uint src, dst;

  opt_mark();

  bp.empty();
  src = dst = 0;
  while ((i = get_instr(src))) {
    if (!i->opt_is_marked()) {
      ::destroy(i);
      src += 1;
    } else {
      if (src != dst) {
        m_instructions[dst] = i;

        /* Move the instruction and update prev. jumps */
        sp_branch_instr *ibp;
        List_iterator_fast<sp_branch_instr> li(bp);

        while ((ibp = li++)) ibp->set_destination(src, dst);
      }
      i->opt_move(dst, &bp);
      src += 1;
      dst += 1;
    }
  }

  m_instructions.resize(dst);
  bp.empty();
}

void sp_head::add_mark_lead(uint ip, List<sp_instr> *leads) {
  sp_instr *i = get_instr(ip);

  if (i && !i->opt_is_marked()) leads->push_front(i);
}

void sp_head::opt_mark() {
  uint ip;
  sp_instr *i;
  List<sp_instr> leads;

  /*
    Forward flow analysis algorithm in the instruction graph:
    - first, add the entry point in the graph (the first instruction) to the
      'leads' list of paths to explore.
    - while there are still leads to explore:
      - pick one lead, and follow the path forward. Mark instruction reached.
        Stop only if the end of the routine is reached, or the path converge
        to code already explored (marked).
      - while following a path, collect in the 'leads' list any fork to
        another path (caused by conditional jumps instructions), so that these
        paths can be explored as well.
  */

  /* Add the entry point */
  i = get_instr(0);
  leads.push_front(i);

  /* For each path of code ... */
  while (leads.elements != 0) {
    i = leads.pop();

    /* Mark the entire path, collecting new leads. */
    while (i && !i->opt_is_marked()) {
      ip = i->opt_mark(this, &leads);
      i = get_instr(ip);
    }
  }
}

#ifndef DBUG_OFF
bool sp_head::show_routine_code(THD *thd) {
  Protocol *protocol = thd->get_protocol();
  char buff[2048];
  String buffer(buff, sizeof(buff), system_charset_info);
  List<Item> field_list;
  sp_instr *i;
  bool full_access;
  bool res = false;
  uint ip;

  if (check_show_access(thd, &full_access) || !full_access) return true;

  field_list.push_back(new Item_uint(NAME_STRING("Pos"), 0, 9));
  // 1024 is for not to confuse old clients
  field_list.push_back(new Item_empty_string(
      "Instruction", std::max<size_t>(buffer.length(), 1024U)));
  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    return true;

  for (ip = 0; (i = get_instr(ip)); ip++) {
    /*
      Consistency check. If these are different something went wrong
      during optimization.
    */
    if (ip != i->get_ip()) {
      char tmp[64 + 2 * MY_INT32_NUM_DECIMAL_DIGITS];
      snprintf(tmp, sizeof(tmp), "Instruction at position %u has m_ip=%u", ip,
               i->get_ip());
      /*
        Since this is for debugging purposes only, we don't bother to
        introduce a special error code for it.
      */
      push_warning(thd, Sql_condition::SL_WARNING, ER_UNKNOWN_ERROR, tmp);
    }
    protocol->start_row();
    protocol->store((longlong)ip);

    buffer.set("", 0, system_charset_info);
    i->print(thd, &buffer);
    protocol->store_string(buffer.ptr(), buffer.length(), system_charset_info);
    if ((res = protocol->end_row())) break;
  }

  if (!res) my_eof(thd);

  return res;
}
#endif  // ifndef DBUG_OFF

bool sp_head::merge_table_list(THD *thd, TABLE_LIST *table,
                               LEX *lex_for_tmp_check) {
  if (lex_for_tmp_check->sql_command == SQLCOM_DROP_TABLE &&
      lex_for_tmp_check->drop_temporary)
    return true;

  for (auto &key_and_value : m_sptabs) {
    key_and_value.second->query_lock_count = 0;
  }

  for (; table; table = table->next_global)
    if (!table->is_internal() && !table->schema_table) {
      /* Fail if this is an inaccessible DD table. */
      const dd::Dictionary *dictionary = dd::get_dictionary();
      if (dictionary &&
          !dictionary->is_dd_table_access_allowed(
              thd->is_dd_system_thread(),
              table->mdl_request.is_ddl_or_lock_tables_lock_request(),
              table->db, table->db_length, table->table_name)) {
        my_error(ER_NO_SYSTEM_TABLE_ACCESS, MYF(0),
                 ER_THD_NONCONST(thd, dictionary->table_type_error_code(
                                          table->db, table->table_name)),
                 table->db, table->table_name);
        return true;
      }

      /*
        Structure of key for the multi-set is "db\0table\0alias\0".
        Since "alias" part can have arbitrary length we use String
        object to construct the key. By default String will use
        buffer allocated on stack with NAME_LEN bytes reserved for
        alias, since in most cases it is going to be smaller than
        NAME_LEN bytes.
      */
      char tname_buff[(NAME_LEN + 1) * 3];
      String tname(tname_buff, sizeof(tname_buff), &my_charset_bin);
      size_t temp_table_key_length;

      tname.length(0);
      tname.append(table->db, table->db_length);
      tname.append('\0');
      tname.append(table->table_name, table->table_name_length);
      tname.append('\0');
      temp_table_key_length = tname.length();
      tname.append(table->alias);
      tname.append('\0');

      /*
        We ignore alias when we check if table was already marked as temporary
        (and therefore should not be prelocked). Otherwise we will erroneously
        treat table with same name but with different alias as non-temporary.
      */

      SP_TABLE *tab;

      if ((tab = find_or_nullptr(m_sptabs,
                                 std::string(tname.ptr(), tname.length()))) ||
          ((tab = find_or_nullptr(
                m_sptabs, std::string(tname.ptr(), temp_table_key_length))) &&
           tab->temp)) {
        if (tab->lock_type < table->lock_descriptor().type)
          tab->lock_type =
              table->lock_descriptor()
                  .type;  // Use the table with the highest lock type
        tab->query_lock_count++;
        if (tab->query_lock_count > tab->lock_count) tab->lock_count++;
        tab->trg_event_map |= table->trg_event_map;
      } else {
        if (!(tab = (SP_TABLE *)thd->mem_calloc(sizeof(SP_TABLE))))
          return false;
        if (lex_for_tmp_check->sql_command == SQLCOM_CREATE_TABLE &&
            lex_for_tmp_check->query_tables == table &&
            lex_for_tmp_check->create_info->options & HA_LEX_CREATE_TMP_TABLE) {
          tab->temp = true;
          tab->qname.length = temp_table_key_length;
        } else
          tab->qname.length = tname.length();
        tab->qname.str = (char *)thd->memdup(tname.ptr(), tab->qname.length);
        if (!tab->qname.str) return false;
        tab->table_name_length = table->table_name_length;
        tab->db_length = table->db_length;
        tab->lock_type = table->lock_descriptor().type;
        tab->lock_count = tab->query_lock_count = 1;
        tab->trg_event_map = table->trg_event_map;
        if (!m_sptabs.emplace(to_string(tab->qname), tab).second) return false;
      }
    }
  return true;
}

void sp_head::add_used_tables_to_table_list(THD *thd,
                                            TABLE_LIST ***query_tables_last_ptr,
                                            enum_sql_command sql_command,
                                            TABLE_LIST *belong_to_view) {
  /*
    Use persistent arena for table list allocation to be PS/SP friendly.
    Note that we also have to copy database/table names and alias to PS/SP
    memory since current instance of sp_head object can pass away before
    next execution of PS/SP for which tables are added to prelocking list.
    This will be fixed by introducing of proper invalidation mechanism
    once new TDC is ready.
  */
  Prepared_stmt_arena_holder ps_arena_holder(thd);

  for (SP_TABLE *stab : m_sptabs_sorted) {
    if (stab->temp || stab->lock_type == TL_IGNORE) continue;

    char *tab_buff = static_cast<char *>(
        thd->alloc(ALIGN_SIZE(sizeof(TABLE_LIST)) * stab->lock_count));
    char *key_buff =
        static_cast<char *>(thd->memdup(stab->qname.str, stab->qname.length));
    if (!tab_buff || !key_buff) return;

    for (uint j = 0; j < stab->lock_count; j++) {
      /*
        Since we don't allow DDL on base tables in prelocked mode it
        is safe to infer the type of metadata lock from the type of
        table lock.
      */
      enum_mdl_type mdl_lock_type;

      if (sql_command == SQLCOM_LOCK_TABLES) {
        /*
          We are building a table list for LOCK TABLES. We need to
          acquire "strong" locks to ensure that LOCK TABLES properly
          works for storage engines which don't use THR_LOCK locks.
        */
        mdl_lock_type = (stab->lock_type >= TL_WRITE_ALLOW_WRITE)
                            ? MDL_SHARED_NO_READ_WRITE
                            : MDL_SHARED_READ_ONLY;
      } else {
        /*
          For other statements "normal" locks can be acquired.
          Let us respect explicit LOW_PRIORITY clause if was used
          in the routine.
        */
        mdl_lock_type = mdl_type_for_dml(stab->lock_type);
      }

      TABLE_LIST *table = new (tab_buff) TABLE_LIST(
          key_buff, stab->db_length, key_buff + stab->db_length + 1,
          stab->table_name_length,
          key_buff + stab->db_length + 1 + stab->table_name_length + 1,
          stab->lock_type, mdl_lock_type);

      table->is_system_view = dd::get_dictionary()->is_system_view_name(
          table->db, table->table_name);
      table->cacheable_table = true;
      table->prelocking_placeholder = true;
      table->belong_to_view = belong_to_view;
      table->trg_event_map = stab->trg_event_map;
      table->disable_sql_log_bin_triggers =
          !thd->variables.sql_log_bin_triggers;
      /* Everyting else should be zeroed */

      **query_tables_last_ptr = table;
      table->prev_global = *query_tables_last_ptr;
      *query_tables_last_ptr = &table->next_global;

      tab_buff += ALIGN_SIZE(sizeof(TABLE_LIST));
    }
  }
}

bool sp_head::check_show_access(THD *thd, bool *full_access) {
  /*
    Check if user has full access to the routine properties (i.e including
    stored routine code), or partial access (i.e to view its other properties).
  */

  *full_access = has_full_view_routine_access(thd, m_db.str, m_definer_user.str,
                                              m_definer_host.str);
  return *full_access ? false
                      : !has_partial_view_routine_access(
                            thd, m_db.str, m_name.str,
                            m_type == enum_sp_type::PROCEDURE);
}

bool sp_head::set_security_ctx(THD *thd, Security_context **save_ctx) {
  *save_ctx = nullptr;
  LEX_CSTRING definer_user = {m_definer_user.str, m_definer_user.length};
  LEX_CSTRING definer_host = {m_definer_host.str, m_definer_host.length};

  if (m_chistics->suid != SP_IS_NOT_SUID &&
      m_security_ctx.change_security_context(thd, definer_user, definer_host,
                                             m_db.str, save_ctx)) {
    return true;
  }

  /*
    If we changed context to run as another user, we need to check the
    access right for the new context again as someone may have revoked
    the right to use the procedure from this user.
  */

  if (*save_ctx &&
      check_routine_access(thd, EXECUTE_ACL, m_db.str, m_name.str,
                           m_type == enum_sp_type::PROCEDURE, false)) {
    m_security_ctx.restore_security_context(thd, *save_ctx);
    *save_ctx = nullptr;
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////
// sp_parser_data implementation.
///////////////////////////////////////////////////////////////////////////

void sp_parser_data::start_parsing_sp_body(THD *thd, sp_head *sp) {
  m_saved_memroot = thd->mem_root;
  m_saved_item_list = thd->item_list();

  thd->mem_root = sp->get_persistent_mem_root();
  thd->mem_root->set_max_capacity(m_saved_memroot->get_max_capacity());
  thd->mem_root->set_error_for_capacity_exceeded(
      m_saved_memroot->get_error_for_capacity_exceeded());
  thd->reset_item_list();
}

void sp_parser_data::finish_parsing_sp_body(THD *thd) {
  /*
    In some cases the parser detects a syntax error and calls
    THD::cleanup_after_parse_error() method only after finishing parsing
    the whole routine. In such a situation sp_head::restore_thd_mem_root()
    will be called twice - the first time as part of normal parsing process
    and the second time by cleanup_after_parse_error().

    To avoid ruining active arena/mem_root state in this case we skip
    restoration of old arena/mem_root if this method has been already called
    for this routine.
  */
  if (!is_parsing_sp_body()) return;

  thd->free_items();
  thd->mem_root = m_saved_memroot;
  thd->set_item_list(m_saved_item_list);

  m_saved_memroot = nullptr;
  m_saved_item_list = nullptr;
}

bool sp_parser_data::add_backpatch_entry(sp_branch_instr *i, sp_label *label) {
  Backpatch_info *bp =
      (Backpatch_info *)(*THR_MALLOC)->Alloc(sizeof(Backpatch_info));

  if (!bp) return true;

  bp->label = label;
  bp->instr = i;
  return m_backpatch.push_front(bp);
}

void sp_parser_data::do_backpatch(sp_label *label, uint dest) {
  Backpatch_info *bp;
  List_iterator_fast<Backpatch_info> li(m_backpatch);

  while ((bp = li++)) {
    if (bp->label == label) bp->instr->backpatch(dest);
  }
}

bool sp_parser_data::add_cont_backpatch_entry(sp_lex_branch_instr *i) {
  i->set_cont_dest(m_cont_level);
  return m_cont_backpatch.push_front(i);
}

void sp_parser_data::do_cont_backpatch(uint dest) {
  sp_lex_branch_instr *i;

  while ((i = m_cont_backpatch.head()) && i->get_cont_dest() == m_cont_level) {
    i->set_cont_dest(dest);
    m_cont_backpatch.pop();
  }

  --m_cont_level;
}

void sp_parser_data::process_new_sp_instr(THD *thd, sp_instr *i) {
  /*
    thd->m_item_list should be cleaned here because it's implicitly expected
    that that process_new_sp_instr() (called from sp_head::add_instr) is
    called as the last action after parsing the SP-instruction's SQL query.

    Thus, at this point THD's item list contains all Item-objects, created for
    this SP-instruction.

    Next SP-instruction should start its own free-list from the scratch.
  */

  i->m_arena.set_item_list(thd->item_list());

  thd->reset_item_list();
}

Stored_program_creation_ctx::Stored_program_creation_ctx(THD *thd)
    : Default_object_creation_ctx(thd),
      m_db_cl(thd->variables.collation_database) {}

void Stored_program_creation_ctx::change_env(THD *thd) const {
  thd->variables.collation_database = m_db_cl;
  Default_object_creation_ctx::change_env(thd);
}
