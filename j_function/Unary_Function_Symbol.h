#ifndef UNARY_FUNCTION_SYMBOL_H
#define UNARY_FUNCTION_SYMBOL_H

#include "j_routine_symbol.h"
//
#include <string>
//
#include <functional>
//
#include "J_Symbol_Error.h"
namespace jomike{

template<typename Ret_t, typename Arg_t>
class Unary_Function_Symbol : public j_routine_symbol{
public:
	template<typename Func_t>
	Unary_Function_Symbol(Func_t i_function, const std::string& irk_string);

	Unary_Function_Symbol* move_copy()override{
		return  new Unary_Function_Symbol(std::move(*this));
	}

	Unary_Function_Symbol* get_copy()const override{
		return new Unary_Function_Symbol(*this);
	}

private:
	std::function<Ret_t(Arg_t)> M_function;
	J_UI_String M_name;

	j_value derived_get_value(const Arguments& i_args)const override;
};

template<typename Ret_t, typename Arg_t>
template<typename Func_t>
Unary_Function_Symbol<Ret_t, Arg_t>::Unary_Function_Symbol(
	Func_t i_function, const std::string& irk_name)
	:j_routine_symbol(new J_Symbol_Identifier(irk_name)
	, new Type_Routine(
		make_type_syntax(CPP_To_Symbol_Type<Ret_t>::type())
		, new Type_Syntax_List({make_type_syntax(CPP_To_Symbol_Type<Arg_t>::type())})))
	, M_function(i_function), M_name(irk_name){
	set_args(Arguments(1));
}


template<typename Ret_t, typename Arg_t>
j_value Unary_Function_Symbol<Ret_t, Arg_t>
	::derived_get_value(const Arguments& i_args)const {
		if(1 != i_args.size()){
			throw J_Sym_Argument_Error("To many Arguments to: " + name().std_str());
		}
		return j_value(M_function(i_args[0].get_value().as_type<Arg_t>()), J_Unit());
}


}

#endif //UNARY_FUNCTION_SYMBOL_H


