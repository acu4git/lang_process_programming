#ifndef PARSER_H
#define PARSER_H

#define NORMAL 0
#define ERROR -1

extern int token;

static int parse_program();                  // プログラム
static int parse_block();                    // ブロック
static int parse_variable_declaration();     // 変数宣言部
static int parse_variable_names();           // 変数名の並び
static int parse_variable_name();            // 変数名
static int parse_type();                     // 型
static int parse_standard_type();            // 標準型
static int parse_array_type();               // 配列型
static int parse_subprogram_declaration();   // 副プログラム宣言
static int parse_procedure_name();           // 手続き名
static int parse_formal_parameters();        // 仮引数名
static int parse_compound_statement();       // 複合文
static int parse_statement();                // 文
static int parse_condition_statement();      // 分岐文
static int parse_iteration_statement();      // 繰り返し文
static int parse_exit_statement();           // 脱出文
static int parse_call_statement();           // 手続き呼び出し文
static int parse_expressions();              // 式の並び
static int parse_return_statement();         // 戻り文
static int parse_assignment_statement();     // 代入文
static int parse_left_part();                // 左辺部
static int parse_variable();                 // 変数
static int parse_expression();               // 式
static int parse_simple_expression();        // 単純式
static int parse_term();                     // 項
static int parse_factor();                   // 因子
static int parse_constant();                 // 定数
static int parse_multiplicative_operator();  // 乗法演算子
static int parse_madditive_operator();       // 加法演算子
static int parse_relational_operator();      // 関係演算子
static int parse_input_statement();          // 入力文
static int parse_output_statement();         // 出力文
static int parse_empty_statement();          // 空文

#endif