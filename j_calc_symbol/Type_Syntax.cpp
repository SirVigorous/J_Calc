#include "Type_Syntax.h"


namespace jomike{



Type_Syntax::Type_Syntax(const yy::location& irk_loc
	, const jc_string_t& irk_name, Symbol_Types i_symbol_type)
	:j_calc_symbol_component(irk_loc, i_symbol_type), M_type_name(irk_name){

}


j_value Type_Syntax::convert_value(const j_value& irk_val)const{
	if(irk_val.symbol_type() == symbol_type()){
		return irk_val;
	}

	assert(!"Did Not implement conversion for this type");
	return j_value();
}

const jc_string_t& Type_Syntax::type_name()const{
	return M_type_name;
}

bool Type_Syntax::is_same_type(const Type_Syntax& irk_right)const{
	return type_name() == irk_right.type_name();
}

bool Type_Syntax::operator==(const Type_Syntax& irk_right)const{
	return is_same_type(irk_right);
}

bool Type_Syntax::operator!=(const Type_Syntax& irk_right)const{
	return !(*this == irk_right);
}

bool Type_Syntax::is_void()const{
	return false;
}

}



