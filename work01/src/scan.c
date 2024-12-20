#include "scan.h"

static FILE* fp;
static int cbuf;
static char errmsg[MAXSTRSIZE] = "";
static int linenum;

int num_attr;
char string_attr[MAXSTRSIZE] = "";
char string_attr_buff[MAXSTRSIZE] = "";

int init_scan(char* filename) {
  if ((fp = fopen(filename, "r")) == NULL) return S_ERROR;
  cbuf = fgetc(fp);
  linenum = 0;
  num_attr = -1;
  return 0;
}

void end_scan() { fclose(fp); }

int scan() {
  if (cbuf == EOF) return -1;

  int ret = 0;

  if (isalpha(cbuf)) {  // name or keyword
    strncpy(string_attr_buff, string_attr, MAXSTRSIZE);
    clear_string_attr();
    do {
      ret = push_char(cbuf);
      if (ret == -1) return S_ERROR;
      cbuf = fgetc(fp);
    } while (isalnum(cbuf));

    ret = is_keyword(string_attr);
    if (ret == -1) {
      return TNAME;
    } else {
      clear_string_attr();
      strncpy(string_attr, string_attr_buff, MAXSTRSIZE);
      return ret;
    }
  } else if (isdigit(cbuf)) {  // number
    clear_string_attr();
    do {
      ret = push_char(cbuf);
      if (ret == -1) return S_ERROR;
      cbuf = fgetc(fp);
    } while (isdigit(cbuf));
    num_attr = atoi(string_attr);
    if (num_attr > 32768) return error("number must not be larger than 32768");
    return TNUMBER;
  } else {
    switch (cbuf) {
      // blank
      case ' ':
      case '\t':
        skip_blank();
        if (cbuf == EOF) return -1;
        return 0;
      // comment
      case '{':
        skip_bracket_comment();
        return 0;
      case '/':
        cbuf = fgetc(fp);
        switch (cbuf) {
          case '*':
            skip_slash_comment();
            return 0;
          default:
            snprintf(errmsg, MAXSTRSIZE, "cannot scan \'%c\'(ascii: %d)", cbuf,
                     cbuf);
            error(errmsg);
            return S_ERROR;
        }
      // string
      case '\'':
        clear_string_attr();
        while (1) {
          switch (cbuf = fgetc(fp)) {
            case EOF:
              return error("found an invalid apostorphy(\')");
            case '\'':
              ret = push_char(cbuf);
              if (ret == -1) return S_ERROR;
              switch (cbuf = fgetc(fp)) {
                case '\'':
                  ret = push_char(cbuf);
                  if (ret == -1) return S_ERROR;
                  break;
                default:
                  ret = pop_char();
                  if (ret == -1) return S_ERROR;
                  return TSTRING;
              }
              break;
            case '\n':
              cbuf = fgetc(fp);
              switch (cbuf) {
                case '\r':  // LF+CR ???
                  cbuf = fgetc(fp);
                  break;
                default:  // LF
              }
              return error("found a line break when scanning a string");
            case '\r':
              cbuf = fgetc(fp);
              switch (cbuf) {
                case '\n':  // CR+LF
                  cbuf = fgetc(fp);
                  break;
                default:  // CR
              }
              return error("found a line break when scanning a string");
            default:
              ret = push_char(cbuf);
              if (ret == -1) return S_ERROR;
              break;
          }
        }
      // symbol
      case '+':
        cbuf = fgetc(fp);
        return TPLUS;
      case '-':
        cbuf = fgetc(fp);
        return TMINUS;
      case '*':
        cbuf = fgetc(fp);
        return TSTAR;
      case '=':
        cbuf = fgetc(fp);
        return TEQUAL;
      case '<':
        cbuf = fgetc(fp);
        switch (cbuf) {
          case '>':
            cbuf = fgetc(fp);
            return TNOTEQ;
          case '=':
            cbuf = fgetc(fp);
            return TLEEQ;
          default:
            ungetc(cbuf, fp);
            return TLE;
        }
      case '>':
        cbuf = fgetc(fp);
        switch (cbuf) {
          case '=':
            cbuf = fgetc(fp);
            return TGREQ;
          default:
            return TGR;
        }
      case '(':
        cbuf = fgetc(fp);
        return TLPAREN;
      case ')':
        cbuf = fgetc(fp);
        return TRPAREN;
      case '[':
        cbuf = fgetc(fp);
        return TLSQPAREN;
      case ']':
        cbuf = fgetc(fp);
        return TRSQPAREN;
      case ':':
        cbuf = fgetc(fp);
        switch (cbuf) {
          case '=':
            cbuf = fgetc(fp);
            return TASSIGN;
          default:
            return TCOLON;
        }
      case ';':
        cbuf = fgetc(fp);
        return TSEMI;
      case '.':
        cbuf = fgetc(fp);
        return TDOT;
      case ',':
        cbuf = fgetc(fp);
        return TCOMMA;
      // line break
      case '\n':
        cbuf = fgetc(fp);
        switch (cbuf) {
          case '\r':  // LF+CR ???
            cbuf = fgetc(fp);
            break;
          default:  // LF
        }
        linenum++;
        return 0;
      case '\r':
        cbuf = fgetc(fp);
        switch (cbuf) {
          case '\n':  // CR+LF
            cbuf = fgetc(fp);
            break;
          default:  // CR
        }
        linenum++;
        return 0;
      default:
        snprintf(errmsg, MAXSTRSIZE, "cannot scan \'%c\'(ascii: %d)", cbuf,
                 cbuf);
        error(errmsg);
        return S_ERROR;
    }
  }
}

int push_char(char c) {
  int len = strlen(string_attr);
  if (len >= MAXSTRSIZE - 1) {
    error("cannot append a character to str array");
    return -1;
  }
  string_attr[len] = c;
  string_attr[len + 1] = '\0';
  return 0;
}

int pop_char() {
  int res = 0;
  int len = strlen(string_attr);
  if (len <= 0) {
    error("cannot pop a character from str array");
    return -1;
  }

  res = string_attr[len - 1];
  string_attr[len - 1] = '\0';
  return res;
}

void clear_string_attr() {
  for (int i = 0; i < MAXSTRSIZE; i++) {
    string_attr[i] = '\0';
  }
}

int is_keyword(char* str) {
  for (int i = 0; i < KEYWORDSIZE; i++) {
    if (strcmp(str, key[i].keyword) == 0) return key[i].keytoken;
  }
  return -1;
}

int get_linenum() { return linenum; }

void skip_blank() {
  while (cbuf != EOF && (cbuf == ' ' || cbuf == '\t')) cbuf = fgetc(fp);
}

void skip_bracket_comment() {
  cbuf = fgetc(fp);
  while (cbuf != EOF && cbuf != '}') {
    switch (cbuf) {
      case '\n':
        cbuf = fgetc(fp);
        switch (cbuf) {
          case '\r':  // LF+CR ???
            cbuf = fgetc(fp);
            break;
          default:  // LF
        }
        linenum++;
        break;
      case '\r':
        cbuf = fgetc(fp);
        switch (cbuf) {
          case '\n':  // CR+LF
            cbuf = fgetc(fp);
            break;
          default:  // CR
        }
        linenum++;
      default:
        cbuf = fgetc(fp);
    }
  }

  if (cbuf == EOF) return;
  if (cbuf == '}') cbuf = fgetc(fp);
}

void skip_slash_comment() {
  cbuf = fgetc(fp);
  while (cbuf != EOF) {
    switch (cbuf) {
      case '*':
        cbuf = fgetc(fp);
        if (cbuf == '/') {
          cbuf = fgetc(fp);
          return;
        }
      default:
        cbuf = fgetc(fp);
        continue;
    }
  }
}