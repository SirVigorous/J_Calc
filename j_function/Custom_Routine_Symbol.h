#ifndef CUSTOM_ROUTINE_SYMBOL_H
#define CUSTOM_ROUTINE_SYMBOL_H

#include "j_routine_symbol.h"


namespace jomike{
class Custom_Routine_Symbol : public j_routine_symbol{
public:
	Custom_Routine_Symbol(
		J_Symbol_Identifier* i_identifier, const Declaration_List& irk_static_declarations
		, const Declaration_List& irk_arg_declarations, Type_Syntax* i_return_type
		, Statement_Block* i_statement_block);

	Custom_Routine_Symbol(const Custom_Routine_Symbol& irk_right);
	Custom_Routine_Symbol(Custom_Routine_Symbol&& irv_right);


	Custom_Routine_Symbol& operator=(const Custom_Routine_Symbol&) = delete;
	Custom_Routine_Symbol& operator=(Custom_Routine_Symbol&&) = delete;

	Custom_Routine_Symbol* get_copy()const override;

	Custom_Routine_Symbol* move_copy()override;

	~Custom_Routine_Symbol();

private:

	j_value derived_get_value(const Arguments& i_args)const override;

	ex_array<J_UI_String> M_arg_names;

	Statement_Block* M_statement_block;
	J_Symbol_Scope* M_running_scope;
};

}

#endif //CUSTOM_ROUTINE_SYMBOL_H


