#include "scan.h"

static FILE* fp;
static int cbuf;
static char errmsg[MAXSTRSIZE] = "";
static int linenum = 1;
static int token_linenum = 0;

int num_attr = -1;
char string_attr[MAXSTRSIZE] = "";
char string_attr_buff[MAXSTRSIZE] = "";
int string_attr_len = 0;

int init_scan(char* filename) {
  if ((fp = fopen(filename, "r")) == NULL) return S_ERROR;
  cbuf = fgetc(fp);
  return 0;
}

void end_scan() { fclose(fp); }

int scan() {
  while (1) {
    if (cbuf == EOF) return -1;

    int ret = -1;
    if (cbuf == ' ' || cbuf == '\t') {  // blank
      skip_blank();
    } else if (cbuf == '{') {  // comment
      skip_bracket_comment();
    } else if (cbuf == '/') {
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
    } else if (cbuf == '\r' || cbuf == '\n') {  // line break
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
          break;
      }
    } else if (isalpha(cbuf)) {  // name or keyword
      strncpy(string_attr_buff, string_attr, MAXSTRSIZE);
      clear_string_attr();
      do {
        ret = push_char(cbuf);
        if (ret == -1) return S_ERROR;
        cbuf = fgetc(fp);
      } while (isalnum(cbuf));

      ret = is_keyword(string_attr);
      if (ret == -1) {
        token_linenum = linenum;
        return TNAME;
      } else {
        clear_string_attr();
        strncpy(string_attr, string_attr_buff, MAXSTRSIZE);
        token_linenum = linenum;
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
      token_linenum = linenum;
      return TNUMBER;
    } else {
      switch (cbuf) {
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
                    string_attr_len--;  // ('')で1文字であるため
                    break;
                  default:
                    ret = pop_char();
                    if (ret == -1) return S_ERROR;
                    token_linenum = linenum;
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
          token_linenum = linenum;
          return TPLUS;
        case '-':
          cbuf = fgetc(fp);
          token_linenum = linenum;
          return TMINUS;
        case '*':
          cbuf = fgetc(fp);
          token_linenum = linenum;
          return TSTAR;
        case '=':
          cbuf = fgetc(fp);
          token_linenum = linenum;
          return TEQUAL;
        case '<':
          cbuf = fgetc(fp);
          switch (cbuf) {
            case '>':
              cbuf = fgetc(fp);
              token_linenum = linenum;
              return TNOTEQ;
            case '=':
              cbuf = fgetc(fp);
              token_linenum = linenum;
              return TLEEQ;
            default:
              ungetc(cbuf, fp);
              token_linenum = linenum;
              return TLE;
          }
        case '>':
          cbuf = fgetc(fp);
          switch (cbuf) {
            case '=':
              cbuf = fgetc(fp);
              token_linenum = linenum;
              return TGREQ;
            default:
              token_linenum = linenum;
              return TGR;
          }
        case '(':
          cbuf = fgetc(fp);
          token_linenum = linenum;
          return TLPAREN;
        case ')':
          cbuf = fgetc(fp);
          token_linenum = linenum;
          return TRPAREN;
        case '[':
          cbuf = fgetc(fp);
          token_linenum = linenum;
          return TLSQPAREN;
        case ']':
          cbuf = fgetc(fp);
          token_linenum = linenum;
          return TRSQPAREN;
        case ':':
          cbuf = fgetc(fp);
          switch (cbuf) {
            case '=':
              cbuf = fgetc(fp);
              token_linenum = linenum;
              return TASSIGN;
            default:
              token_linenum = linenum;
              return TCOLON;
          }
        case ';':
          cbuf = fgetc(fp);
          token_linenum = linenum;
          return TSEMI;
        case '.':
          cbuf = fgetc(fp);
          token_linenum = linenum;
          return TDOT;
        case ',':
          cbuf = fgetc(fp);
          token_linenum = linenum;
          return TCOMMA;
        default:
          snprintf(errmsg, MAXSTRSIZE, "cannot scan \'%c\'(ascii: %d)", cbuf,
                   cbuf);
          error(errmsg);
          return S_ERROR;
      }
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
  string_attr_len++;
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
  string_attr_len--;
  return res;
}

void clear_string_attr() {
  for (int i = 0; i < MAXSTRSIZE; i++) {
    string_attr[i] = '\0';
  }
  string_attr_len = 0;
}

int get_string_attr_len() { return string_attr_len; }

int is_keyword(char* str) {
  for (int i = 0; i < KEYWORDSIZE; i++) {
    if (strcmp(str, key[i].keyword) == 0) return key[i].keytoken;
  }
  return -1;
}

int get_linenum() { return token_linenum; }

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