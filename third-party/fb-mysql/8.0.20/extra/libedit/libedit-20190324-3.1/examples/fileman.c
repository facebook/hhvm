/* fileman.c -- A tiny application which demonstrates how to use the
   GNU Readline library.  This application interactively allows users
   to manipulate files and their modes.

   NOTE: this was taken from the GNU Readline documentation and ported
   to libedit. A command to output the history list was added.
   
   */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <time.h>

/* GNU readline
#include <readline/readline.h>
#include <readline/history.h>
*/
#include <editline/readline.h>

void * xmalloc (size_t size);
void too_dangerous (char *caller);
void initialize_readline ();
int execute_line (char *line);
int valid_argument (char *caller, char *arg);

typedef int rl_icpfunc_t (char *);

/* The names of functions that actually do the manipulation. */
int com_list (char *);
int com_view (char *);
int com_history (char *);
int com_rename(char *);
int com_stat(char *);
int com_pwd(char *);
int com_delete(char *);
int com_help(char *);
int com_cd(char *);
int com_quit(char *);

/* A structure which contains information on the commands this program
   can understand. */

typedef struct {
   char *name;                   /* User printable name of the function. */
   rl_icpfunc_t *func;           /* Function to call to do the job. */
   char *doc;                    /* Documentation for this function.  */
} COMMAND;

COMMAND commands[] = {
   { "cd", com_cd, "Change to directory DIR" },
   { "delete", com_delete, "Delete FILE" },
   { "help", com_help, "Display this text" },
   { "?", com_help, "Synonym for `help'" },
   { "list", com_list, "List files in DIR" },
   { "ls", com_list, "Synonym for `list'" },
   { "pwd", com_pwd, "Print the current working directory" },
   { "quit", com_quit, "Quit using Fileman" },
   { "rename", com_rename, "Rename FILE to NEWNAME" },
   { "stat", com_stat, "Print out statistics on FILE" },
   { "view", com_view, "View the contents of FILE" },
   { "history", com_history, "List editline history" },
   { (char *)NULL, (rl_icpfunc_t *)NULL, (char *)NULL }
};

/* Forward declarations. */
char *stripwhite ();
COMMAND *find_command ();

/* The name of this program, as taken from argv[0]. */
char *progname;

/* When non-zero, this means the user is done using this program. */
int done;

char *
dupstr (char* s)
{
   char *r;

   r = xmalloc (strlen (s) + 1);
   strcpy (r, s);
   return (r);
}

int
main (int argc __attribute__((__unused__)), char **argv)
{
   char *line, *s;

   progname = argv[0];

   setlocale(LC_CTYPE, "");

   initialize_readline();       /* Bind our completer. */

   stifle_history(7);

   /* Loop reading and executing lines until the user quits. */
   for ( ; done == 0; )
   {
      line = readline ("FileMan: ");

      if (!line)
         break;

      /* Remove leading and trailing whitespace from the line.
         Then, if there is anything left, add it to the history list
         and execute it. */
      s = stripwhite(line);

      if (*s) {

         char* expansion;
         int result;

         result = history_expand(s, &expansion);

         if (result < 0 || result == 2) {
            fprintf(stderr, "%s\n", expansion);
         } else {
            add_history(expansion);
            execute_line(expansion);
         }
         free(expansion);
      }

      free(line);
   }
   exit (0);

   return 0;
}

/* Execute a command line. */
int
execute_line (char *line)
{
   register int i;
   COMMAND *command;
   char *word;

   /* Isolate the command word. */
   i = 0;
   while (line[i] && isspace (line[i]))
      i++;
   word = line + i;

   while (line[i] && !isspace (line[i]))
      i++;

   if (line[i])
      line[i++] = '\0';

   command = find_command (word);

   if (!command)
   {
      fprintf (stderr, "%s: No such command for FileMan.\n", word);
      return (-1);
   }

   /* Get argument to command, if any. */
   while (isspace (line[i]))
      i++;

   word = line + i;

   /* Call the function. */
   return ((*(command->func)) (word));
}

/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */
COMMAND *
find_command (char *name)
{
   register int i;

   for (i = 0; commands[i].name; i++)
      if (strcmp (name, commands[i].name) == 0)
         return (&commands[i]);

   return ((COMMAND *)NULL);
}

/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
char *
stripwhite (char *string)
{
   register char *s, *t;

   for (s = string; isspace (*s); s++)
      ;

   if (*s == 0)
      return (s);

   t = s + strlen (s) - 1;
   while (t > s && isspace (*t))
      t--;
   *++t = '\0';

   return s;
}

/* **************************************************************** */
/*                                                                  */
/*                  Interface to Readline Completion                */
/*                                                                  */
/* **************************************************************** */

char *command_generator(const char *, int);
char **fileman_completion(const char *, int, int);

/* Tell the GNU Readline library how to complete.  We want to try to
   complete on command names if this is the first word in the line, or
   on filenames if not. */
void
initialize_readline ()
{
   /* Allow conditional parsing of the ~/.inputrc file. */
   rl_readline_name = "FileMan";

   /* Tell the completer that we want a crack first. */
   rl_attempted_completion_function = fileman_completion;
}

/* Attempt to complete on the contents of TEXT.  START and END
   bound the region of rl_line_buffer that contains the word to
   complete.  TEXT is the word to complete.  We can use the entire
   contents of rl_line_buffer in case we want to do some simple
   parsing.  Returnthe array of matches, or NULL if there aren't any. */
char **
fileman_completion (const char* text, int start, int end __attribute__((__unused__)))
{
   char **matches;

   matches = (char **)NULL;

   /* If this word is at the start of the line, then it is a command
      to complete.  Otherwise it is the name of a file in the current
      directory. */
   if (start == 0)
      /* TODO */
      matches = completion_matches ((char*)text, command_generator);
      /* matches = rl_completion_matches (text, command_generator); */

   return (matches);
}

/* Generator function for command completion.  STATE lets us
   know whether to start from scratch; without any state
   (i.e. STATE == 0), then we start at the top of the list. */
char *
command_generator (text, state)
   const char *text;
   int state;
{
   static int list_index, len;
   char *name;

   /* If this is a new word to complete, initialize now.  This
      includes saving the length of TEXT for efficiency, and
      initializing the index variable to 0. */
   if (!state)
   {
      list_index = 0;
      len = strlen (text);
   }

   /* Return the next name which partially matches from the
      command list. */
   while ((name = commands[list_index].name))
   {
      list_index++;

      if (strncmp (name, text, len) == 0)
         return (dupstr(name));
   }

   /* If no names matched, then return NULL. */
   return ((char *)NULL);
}

/* **************************************************************** */
/*                                                                  */
/*                       FileMan Commands                           */
/*                                                                  */
/* **************************************************************** */

/* String to pass to system ().  This is for the LIST, VIEW and RENAME
   commands. */
static char syscom[1024];

/* List the file(s) named in arg. */
int
com_list (char *arg)
{
   if (!arg)
      arg = "";

   sprintf (syscom, "ls -FClg %s", arg);
   return (system (syscom));
}

int
com_view (char *arg)
{
   if (!valid_argument ("view", arg))
      return 1;

   sprintf (syscom, "more %s", arg);
   return (system (syscom));
}

int
com_history(char* arg __attribute__((__unused__)))
{
   HIST_ENTRY *he;

   /* rewind history */
   while (previous_history())
      ;

   for (he = current_history(); he != NULL; he = next_history()) {
      //printf("%5d  %s\n", *((int*)he->data) - 1, he->line);
      printf("%s\n", he->line);
   }

   return 0;
}

int
com_rename (char *arg __attribute__((__unused__)))
{
   too_dangerous ("rename");
   return (1);
}

int
com_stat (char *arg)
{
   struct stat finfo;

   if (!valid_argument ("stat", arg))
      return (1);

   if (stat (arg, &finfo) == -1)
   {
      perror (arg);
      return (1);
   }

   printf ("Statistics for `%s':\n", arg);

   printf ("%s has %ld link%s, and is %lld byte%s in length.\n", arg,
         (long) finfo.st_nlink,
         (finfo.st_nlink == 1) ? "" : "s",
         (long long) finfo.st_size,
         (finfo.st_size == 1) ? "" : "s");
   printf ("Inode Last Change at: %s", ctime (&finfo.st_ctime));
   printf ("      Last access at: %s", ctime (&finfo.st_atime));
   printf ("    Last modified at: %s", ctime (&finfo.st_mtime));
   return (0);
}

int
com_delete (char *arg __attribute__((__unused__)))
{
   too_dangerous ("delete");
   return (1);
}

/* Print out help for ARG, or for all of the commands if ARG is
   not present. */
int
com_help (char *arg)
{
   register int i;
   int printed = 0;

   for (i = 0; commands[i].name; i++)
   {
      if (!*arg || (strcmp (arg, commands[i].name) == 0))
      {
         printf ("%s\t\t%s.\n", commands[i].name, commands[i].doc);
         printed++;
      }
   }

   if (!printed)
   {
      printf ("No commands match `%s'.  Possibilties are:\n", arg);

      for (i = 0; commands[i].name; i++)
      {
         /* Print in six columns. */
         if (printed == 6)
         {
            printed = 0;
            printf ("\n");
         }

         printf ("%s\t", commands[i].name);
         printed++;
      }

      if (printed)
         printf ("\n");
   }
   return (0);
}

/* Change to the directory ARG. */
int
com_cd (char *arg)
{
   if (chdir (arg) == -1)
   {
      perror (arg);
      return 1;
   }

   com_pwd ("");
   return (0);
}

/* Print out the current working directory. */
int
com_pwd (char* ignore __attribute__((__unused__)))
{
   char dir[1024], *s;

   s = (char*)getcwd(dir, sizeof(dir) - 1);
   if (s == 0)
   {
      printf ("Error getting pwd: %s\n", dir);
      return 1;
   }

   printf ("Current directory is %s\n", dir);
   return 0;
}

/* The user wishes to quit using this program.  Just set DONE
   non-zero. */
int 
com_quit (char *arg __attribute__((__unused__)))
{
   done = 1;
   return (0);
}

/* Function which tells you that you can't do this. */
void
too_dangerous (char *caller)
{
   fprintf (stderr,
         "%s: Too dangerous for me to distribute.\n",
         caller);
   fprintf (stderr, "Write it yourself.\n");
}

/* Return non-zero if ARG is a valid argument for CALLER,
   else print an error message and return zero. */
int
valid_argument (char *caller, char *arg)
{
   if (!arg || !*arg)
   {
      fprintf (stderr, "%s: Argument required.\n", caller);
      return (0);
   }

   return (1);
}

void *
xmalloc (size_t size)
{
   register void *value = (void*)malloc(size);
   if (value == 0)
      fprintf(stderr, "virtual memory exhausted");
   return value;
}


