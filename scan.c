#include "scan.h"

#include <ctype.h>  //Determination of alphabetic or numeric characters such as isalpha
#include <stdio.h>   //Necessary to use fopen or printf.
#include <stdlib.h>  //Necessary to use malloc or exit
#include <string.h>  //Necessary to use strcpy or strcmp

FILE *fp;  // File pointer. Used to point to which file is being handled and
           // which location in the file is being read or written.
char string_attr[MAXSTRSIZE];  // When the return value of scan() is "name" or
                               // "string", the actual string is stored.
int num_attr = 0;  // When the return value of scan() is an "unsigned integer",
                   // the value is retained.
static int cbuf;   // Holds the character being scanned. Used to determine the
                   // character.
static int next_cbuf = '\0';  // Holds the next character to be scanned. Read
                              // ahead. Initial value is NULL.
static int linenum = 1;       // Hold current line number
static int token_linenum =
    0;  // Holds the line number where the scanned token was located

static int blank(int c);  // Find out if c is a space or a tab
static int alphanum();    // Scan alphabetical or numeric strings
static int digit();       // Scan a string of numbers
static int string();      // Scan string
static int comment();     // Scan comment
static int symbol();      // Scan symbol
static int keyword(
    char *token);  // Determine whether it is a keyword or not and return its
                   // token code if it is a keyword. If it is not a keyword,
                   // treat it as a name token.
static int string_attr_push(const char c);  // Add a token to the string
static void next();        // Function to look ahead to the next character
int get_linenum();         // returns the number of the row when the token most
                           // recently returned by scan() was present
void set_token_linenum();  // set current linenum to token_linenum
int end_scan();            // Close the file

/*open the file represented by the initialization function filename as an input
 * file*/
int init_scan(char *filename) {
  if ((fp = fopen(filename, "r")) == NULL) {
    error("Could not open file at init_scan.");
    return -1;
  }
  next();  // Prepare letters for anticipation by calling twice as well.
  next();
  return 0;
}

int scan() {
  int token_code = -1;  // Variable for storing token code.
  while (1) {
    if (cbuf == EOF) {
      return -1;
    } else if (cbuf == '\r' ||
               cbuf == '\n') {  // when returning to the beginning of a line or
                                // a new line
      if (cbuf == '\r') {
        if (next_cbuf == '\n') {
          next();
        }
        next();
        linenum++;
      } else {
        if (next_cbuf == '\r') {
          next();
        }
        next();
        linenum++;
      }
    } else if (blank(cbuf)) {  // Examine whitespace and tabs
      next();
    } else if (!isprint(cbuf)) {  // When the character is not a displayable
                                  // character
      error("At function scan().");
      return -1;
    } else if (isalpha(cbuf)) {  // Obtain token code if alphabet is detected
      token_code = alphanum();
      break;
    } else if (isdigit(cbuf)) {  // When a number is detected
      token_code = digit();
      break;
    } else if (cbuf == '\'') {  // When a string is detected
      token_code = string();
      break;
    } else if ((cbuf == '/' && next_cbuf == '*') ||
               cbuf == '{') {  // When comment is detected
      if (comment() == -1) {
        break;
      }
    } else {  // When a symbol is detected
      token_code = symbol();
      break;
    }
  }
  set_token_linenum();
  return token_code;
}

/*Spaces and tabs*/
static int blank(int c) {
  if (c == ' ' || c == '\t') {
    return 1;
  } else {
    return 0;
  }
}

/*alphabetic or numeric*/
static int alphanum() {
  memset(string_attr, '\0',
         sizeof(string_attr));  // Initialize all bytes of string_attr with null
                                // characters.
  string_attr[0] = cbuf;
  next();  // Continue reading the text
  while (isalnum(cbuf)) {
    if (string_attr_push(cbuf) == -1) {
      error("At function alphanum().");
      return -1;
    }
    next();
  }
  return keyword(string_attr);  // Return token code
}

/*number*/
static int digit() {
  int num = cbuf - '0';  // To make a character an integer
  memset(string_attr, '\0', sizeof(string_attr));
  string_attr[0] = cbuf;
  next();
  while (isdigit(cbuf)) {
    if (string_attr_push(cbuf) == -1) {
      error("At function digit().");
      return -1;
    }
    num *= 10;          // Raise the digit by one
    num += cbuf - '0';  // Add the characters read as an integer
    next();
  }
  if (num <= MAX_NUM_ATTR) {  // Upper limit check
    num_attr = num;
    return TNUMBER;
  } else {
    error("Overflow.\n");
  }
  return -1;
}

/*string*/
static int string() {
  memset(string_attr, '\0', sizeof(string_attr));
  next();
  while (1) {
    if (cbuf == EOF) {
      error("At function string().");
      return -1;
    }

    if (!isprint(cbuf) ||
        (unsigned char)cbuf > 127) {  // Returns -1 when characters that cannot
                                      // be displayed are detected
      error("At function string()");
      return -1;
    }

    if (cbuf == '\'' && next_cbuf != '\'') {  // When it is not "'".
      break;
    }

    if (cbuf == '\'' && next_cbuf == '\'') {  // When it is "'".
      if (string_attr_push(cbuf) == -1) {
        error("At string() in string_attr_push.");
        return -1;
      }
      next();  // Go to next character
    }

    if (string_attr_push(cbuf) == -1) {
      error("function string()");
      return -1;
    }
    next();  // Go to next character
  }
  next();
  return TSTRING;
}

/*comment*/
static int comment() {
  if (cbuf == '/' && next_cbuf == '*') {
    next();
    next();
    while (cbuf != EOF) {
      if (cbuf == '*' && next_cbuf == '/') {  // Loop until "*/" is found.
        next();
        next();
        return 0;
      }
      next();
    }
  } else if (cbuf == '{') {
    next();
    while (cbuf != EOF) {
      if (cbuf == '}') {  // Loop until "}" is found.
        next();
        return 0;
      }
      next();
    }
  }
  /* EOF */
  if (cbuf != EOF) {  // If the end of the comment could not be found
    error("At function comment().");
  }
  return -1;
}

/*symbol*/
static int symbol() {
  char symbol = cbuf;
  next();
  if (symbol == '+') {
    return TPLUS;
  } else if (symbol == '-') {
    return TMINUS;
  } else if (symbol == '*') {
    return TSTAR;
  } else if (symbol == '=') {
    return TEQUAL;
  } else if (symbol == '<') {
    if (cbuf == '>') {
      next();
      return TNOTEQ;
    } else if (cbuf == '=') {
      next();
      return TLEEQ;
    } else {
      return TLE;
    }
  } else if (symbol == '>') {
    if (cbuf == '=') {
      next();
      return TGREQ;
    } else {
      return TGR;
    }
  } else if (symbol == '(') {
    return TLPAREN;
  } else if (symbol == ')') {
    return TRPAREN;
  } else if (symbol == '[') {
    return TLSQPAREN;
  } else if (symbol == ']') {
    return TRSQPAREN;
  } else if (symbol == ':') {
    if (cbuf == '=') {
      next();
      return TASSIGN;
    } else {
      return TCOLON;
    }
  } else if (symbol == '.') {
    return TDOT;
  } else if (symbol == ',') {
    return TCOMMA;
  } else if (symbol == ';') {
    return TSEMI;
  } else {
    error("Undefined character.");
    return -1;
  }
}

/*keyword(token)*/
static int keyword(char *token) {
  int i;
  for (i = 0; i < KEYWORDSIZE; i++) {
    if (strcmp(token, key[i].keyword) ==
        0) {  // Compare the keywords in the keyword list with the TOKEN read so
              // far. If they match, return 0.
      return key[i].keytoken;
    }
  }
  return TNAME;  // If there are no matching keywords, return as TNAME
}

/*Add a new character to the end of string_attr*/
static int string_attr_push(const char c) {
  int len = strlen(string_attr);
  if (len < MAXSTRSIZE - 1) {  // Add text if maximum size is not reached
    string_attr[len] = c;
    return 0;
  } else {
    error("Overflow at function string_attr_push().");
    return -1;
  }
}

/*Read one character from the file. (Read the next character)*/
static void next() {
  cbuf = next_cbuf;
  next_cbuf = fgetc(fp);
  return;
}

/*Return line number*/
int get_linenum() { return token_linenum; }

/*set current linenum to token_linenum*/
void set_token_linenum() { token_linenum = linenum; }

/*Close the file*/
int end_scan() {
  if (fclose(fp) == EOF) {
    error("At end_scan.(EOF)");
    return -1;
  }
  return 0;
}