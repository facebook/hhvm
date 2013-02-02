
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
#ifndef YYTOKEN_MAP
#define YYTOKEN_MAP enum yytokentype
#define YYTOKEN(num, name) name = num
#endif
   YYTOKEN_MAP {
#ifndef YYTOKEN_MIN
#define YYTOKEN_MIN 258
#endif
     YYTOKEN(258, T_REQUIRE_ONCE),
     YYTOKEN(259, T_REQUIRE),
     YYTOKEN(260, T_EVAL),
     YYTOKEN(261, T_INCLUDE_ONCE),
     YYTOKEN(262, T_INCLUDE),
     YYTOKEN(263, T_LOGICAL_OR),
     YYTOKEN(264, T_LOGICAL_XOR),
     YYTOKEN(265, T_LOGICAL_AND),
     YYTOKEN(266, T_PRINT),
     YYTOKEN(267, T_SR_EQUAL),
     YYTOKEN(268, T_SL_EQUAL),
     YYTOKEN(269, T_XOR_EQUAL),
     YYTOKEN(270, T_OR_EQUAL),
     YYTOKEN(271, T_AND_EQUAL),
     YYTOKEN(272, T_MOD_EQUAL),
     YYTOKEN(273, T_CONCAT_EQUAL),
     YYTOKEN(274, T_DIV_EQUAL),
     YYTOKEN(275, T_MUL_EQUAL),
     YYTOKEN(276, T_MINUS_EQUAL),
     YYTOKEN(277, T_PLUS_EQUAL),
     YYTOKEN(278, T_BOOLEAN_OR),
     YYTOKEN(279, T_BOOLEAN_AND),
     YYTOKEN(280, T_IS_NOT_IDENTICAL),
     YYTOKEN(281, T_IS_IDENTICAL),
     YYTOKEN(282, T_IS_NOT_EQUAL),
     YYTOKEN(283, T_IS_EQUAL),
     YYTOKEN(284, T_IS_GREATER_OR_EQUAL),
     YYTOKEN(285, T_IS_SMALLER_OR_EQUAL),
     YYTOKEN(286, T_SR),
     YYTOKEN(287, T_SL),
     YYTOKEN(288, T_INSTANCEOF),
     YYTOKEN(289, T_UNSET_CAST),
     YYTOKEN(290, T_BOOL_CAST),
     YYTOKEN(291, T_OBJECT_CAST),
     YYTOKEN(292, T_ARRAY_CAST),
     YYTOKEN(293, T_STRING_CAST),
     YYTOKEN(294, T_DOUBLE_CAST),
     YYTOKEN(295, T_INT_CAST),
     YYTOKEN(296, T_DEC),
     YYTOKEN(297, T_INC),
     YYTOKEN(298, T_CLONE),
     YYTOKEN(299, T_NEW),
     YYTOKEN(300, T_EXIT),
     YYTOKEN(301, T_IF),
     YYTOKEN(302, T_ELSEIF),
     YYTOKEN(303, T_ELSE),
     YYTOKEN(304, T_ENDIF),
     YYTOKEN(305, T_LNUMBER),
     YYTOKEN(306, T_DNUMBER),
     YYTOKEN(307, T_STRING),
     YYTOKEN(308, T_STRING_VARNAME),
     YYTOKEN(309, T_VARIABLE),
     YYTOKEN(310, T_NUM_STRING),
     YYTOKEN(311, T_INLINE_HTML),
     YYTOKEN(312, T_CHARACTER),
     YYTOKEN(313, T_BAD_CHARACTER),
     YYTOKEN(314, T_ENCAPSED_AND_WHITESPACE),
     YYTOKEN(315, T_CONSTANT_ENCAPSED_STRING),
     YYTOKEN(316, T_ECHO),
     YYTOKEN(317, T_DO),
     YYTOKEN(318, T_WHILE),
     YYTOKEN(319, T_ENDWHILE),
     YYTOKEN(320, T_FOR),
     YYTOKEN(321, T_ENDFOR),
     YYTOKEN(322, T_FOREACH),
     YYTOKEN(323, T_ENDFOREACH),
     YYTOKEN(324, T_DECLARE),
     YYTOKEN(325, T_ENDDECLARE),
     YYTOKEN(326, T_AS),
     YYTOKEN(327, T_SWITCH),
     YYTOKEN(328, T_ENDSWITCH),
     YYTOKEN(329, T_CASE),
     YYTOKEN(330, T_DEFAULT),
     YYTOKEN(331, T_BREAK),
     YYTOKEN(332, T_GOTO),
     YYTOKEN(333, T_CONTINUE),
     YYTOKEN(334, T_FUNCTION),
     YYTOKEN(335, T_CONST),
     YYTOKEN(336, T_RETURN),
     YYTOKEN(337, T_TRY),
     YYTOKEN(338, T_CATCH),
     YYTOKEN(339, T_THROW),
     YYTOKEN(340, T_USE),
     YYTOKEN(341, T_GLOBAL),
     YYTOKEN(342, T_PUBLIC),
     YYTOKEN(343, T_PROTECTED),
     YYTOKEN(344, T_PRIVATE),
     YYTOKEN(345, T_FINAL),
     YYTOKEN(346, T_ABSTRACT),
     YYTOKEN(347, T_STATIC),
     YYTOKEN(348, T_VAR),
     YYTOKEN(349, T_UNSET),
     YYTOKEN(350, T_ISSET),
     YYTOKEN(351, T_EMPTY),
     YYTOKEN(352, T_HALT_COMPILER),
     YYTOKEN(353, T_CLASS),
     YYTOKEN(354, T_INTERFACE),
     YYTOKEN(355, T_EXTENDS),
     YYTOKEN(356, T_IMPLEMENTS),
     YYTOKEN(357, T_OBJECT_OPERATOR),
     YYTOKEN(358, T_DOUBLE_ARROW),
     YYTOKEN(359, T_LIST),
     YYTOKEN(360, T_ARRAY),
     YYTOKEN(361, T_CLASS_C),
     YYTOKEN(362, T_METHOD_C),
     YYTOKEN(363, T_FUNC_C),
     YYTOKEN(364, T_LINE),
     YYTOKEN(365, T_FILE),
     YYTOKEN(366, T_COMMENT),
     YYTOKEN(367, T_DOC_COMMENT),
     YYTOKEN(368, T_OPEN_TAG),
     YYTOKEN(369, T_OPEN_TAG_WITH_ECHO),
     YYTOKEN(370, T_CLOSE_TAG),
     YYTOKEN(371, T_WHITESPACE),
     YYTOKEN(372, T_START_HEREDOC),
     YYTOKEN(373, T_END_HEREDOC),
     YYTOKEN(374, T_DOLLAR_OPEN_CURLY_BRACES),
     YYTOKEN(375, T_CURLY_OPEN),
     YYTOKEN(376, T_PAAMAYIM_NEKUDOTAYIM),
     YYTOKEN(377, T_NAMESPACE),
     YYTOKEN(378, T_NS_C),
     YYTOKEN(379, T_DIR),
     YYTOKEN(380, T_NS_SEPARATOR),
     YYTOKEN(381, T_YIELD),
     YYTOKEN(382, T_XHP_LABEL),
     YYTOKEN(383, T_XHP_TEXT),
     YYTOKEN(384, T_XHP_ATTRIBUTE),
     YYTOKEN(385, T_XHP_CATEGORY),
     YYTOKEN(386, T_XHP_CATEGORY_LABEL),
     YYTOKEN(387, T_XHP_CHILDREN),
     YYTOKEN(388, T_XHP_ENUM),
     YYTOKEN(389, T_XHP_REQUIRED),
     YYTOKEN(390, T_TRAIT),
     YYTOKEN(391, T_INSTEADOF),
     YYTOKEN(392, T_TRAIT_C),
     YYTOKEN(393, T_VARARG),
     YYTOKEN(394, T_STRICT_ERROR),
     YYTOKEN(395, T_FINALLY),
     YYTOKEN(396, T_XHP_TAG_LT),
     YYTOKEN(397, T_XHP_TAG_GT),
     YYTOKEN(398, T_TYPELIST_LT),
     YYTOKEN(399, T_TYPELIST_GT),
     YYTOKEN(400, T_UNRESOLVED_LT),
     YYTOKEN(401, T_COLLECTION)
   };
#ifndef YYTOKEN_MAX
#define YYTOKEN_MAX 401
#endif

#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif



#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



