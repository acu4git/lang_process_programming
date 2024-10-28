#include "scan.h"

static FILE* fp;
static int curr_ch;
static int cbuf;
static char errmsg[MAXSTRSIZE] = "";
static int linenum;

int num_attr;
char string_attr[MAXSTRSIZE] = "";

int init_scan(char* filename) {
  if ((fp = fopen(filename, "r")) == NULL) return S_ERROR;
  cbuf = fgetc(fp);
  linenum = 0;
  num_attr = -1;
  return 0;
}

void end_scan() { fclose(fp); }

int scan() {
  // space or tab
  skip_blank();
  // comment
  switch (cbuf) {
    case '{':
      skip_bracket_comment();
      break;
    case '/':
      cbuf = fgetc(fp);
      switch (cbuf) {
        case '*':
          skip_slash_comment();
          break;
        default:
          snprintf(errmsg, MAXSTRSIZE, "cannot scan \'%c\'(ascii: %d)", cbuf,
                   cbuf);
          error(errmsg);
          return S_ERROR;
      }
  }
  if (cbuf == EOF) return -1;

  int ret = 0;

  if (isalpha(cbuf)) {  // name or keyword
    do {
      ret = append_char(string_attr, cbuf);
      if (ret == -1) return S_ERROR;
      cbuf = fgetc(fp);
    } while (isalnum(cbuf));

    ret = is_keyword(string_attr);
    if (ret != -1)
      return ret;
    else
      return TNAME;

  } else if (isdigit(cbuf)) {  // number
    do {
      ret = append_char(string_attr, cbuf);
      if (ret == -1) return S_ERROR;
      cbuf = fgetc(fp);
    } while (isdigit(cbuf));
    num_attr = atoi(string_attr);
    return TNUMBER;
  } else {
    // symbol
    switch (cbuf) {
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
            ungetc(cbuf, fp);
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
            ungetc(cbuf, fp);
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

int append_char(char* str, char c) {
  int len = strlen(str);
  if (len >= MAXSTRSIZE - 1) {
    error("cannot append a charcter to str array");
    return -1;
  }
  str[len] = c;
  str[len + 1] = '\0';
  return 0;
}

int is_keyword(char* str) {
  for (int i = 0; i < KEYWORDSIZE; i++) {
    if (strcmp(str, key[i].keyword) == 0) return key[i].keytoken;
  }
  return -1;
}

int get_linenum() { return linenum; }

void break_line() {
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
  }
}

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