%token INT RETURN ID NUM ASSIGN PLUS SEMI
%%
program: decl_list ;
decl_list: decl decl_list ;
decl_list: decl ;
decl: INT ID ASSIGN NUM SEMI ;

