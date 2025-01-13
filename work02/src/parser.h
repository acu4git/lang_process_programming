#ifndef PARSER_H
#define PARSER_H

#define NORMAL 0
#define ERROR -1

int parse_program();                  // プログラム
int parse_block();                    // ブロック
int parse_variable_declaration();     // 変数宣言部
int parse_variable_names();           // 変数名の並び
int parse_variable_name();            // 変数名
int parse_type();                     // 型
int parse_standard_type();            // 標準型
int parse_array_type();               // 配列型
int parse_subprogram_declaration();   // 副プログラム宣言
int parse_procedure_name();           // 手続き名
int parse_formal_parameters();        // 仮引数名
int parse_compound_statement();       // 複合文
int parse_statement();                // 文
int parse_condition_statement();      // 分岐文
int parse_iteration_statement();      // 繰り返し文
int parse_exit_statement();           // 脱出文
int parse_call_statement();           // 手続き呼び出し文
int parse_expressions();              // 式の並び
int parse_return_statement();         // 戻り文
int parse_assignment_statement();     // 代入文
int parse_left_part();                // 左辺部
int parse_variable();                 // 変数
int parse_expression();               // 式
int parse_simple_expression();        // 単純式
int parse_term();                     // 項
int parse_factor();                   // 因子
int parse_constant();                 // 定数
int parse_multiplicative_operator();  // 乗法演算子
int parse_madditive_operator();       // 加法演算子
int parse_relational_operator();      // 関係演算子
int parse_input_statement();          // 入力文
int parse_output_statement();         // 出力文
int parse_empty_statement();          // 空文

#endif