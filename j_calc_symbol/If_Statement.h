#ifndef IF_STATEMENT_H
#define IF_STATEMENT_H

#include "j_statement.h"


namespace jomike{
class If_Statement : public j_statement{
public:
	If_Statement(const yy::location& irk_loc
		, j_calc_expression* i_test_expression, Statement_Block* i_then_body
		, j_statement* i_else_body = nullptr);




	
	//Deleting Constructors and Assignment Operators
	If_Statement(const If_Statement& irk_source);
	If_Statement(If_Statement&& irv_source);
	If_Statement& operator=(const If_Statement&) = delete;
	If_Statement& operator=(If_Statement&&) = delete;

	If_Statement* get_copy()const override;

	If_Statement* move_copy()override;

	void clear()override;

	bool has_value()const override;

	~If_Statement();

	void process()override;

	
private:
	j_value derived_get_value(const Arguments& i_args)const override;

	j_value M_test_value;

	void alert_symbol_scope_set()override;

	//Members:
	j_calc_expression* M_test_expression;
	Statement_Block* M_then_body;
	j_statement* M_else_body = nullptr;
};

}

#endif //IF_STATEMENT_H


