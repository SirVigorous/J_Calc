#include "J_Calc_Data.h"
#include "J_Calc_View.h"
#include <j_function.h>
#include "Math_Input_Box.h"
//
#include <J_Math.h>
//Utilities
#include <cassert>
//
#include <j_routine_symbol.h>
//
#include <Binary_Function_Chain_Symbol.h>
//
#include <J_Symbol_Identifier.h>
using std::dynamic_pointer_cast;

using std::string;

namespace jomike{

j_symbol* get_j_symbol(const J_UI_String& irk_name){
	static Instance_Pointer<J_Calc_Data> s_data;
	s_data->get_symbol(irk_name);
}

class GCD_Symbol : public j_routine_wrapper_symbol{
public:
	GCD_Symbol(): j_routine_wrapper_symbol(new J_Symbol_Identifier("gcd"), Symbol_Types::INT){}
	GCD_Symbol* get_copy()const override;
	GCD_Symbol* move_copy()override;
	J_UI_String get_display_name()override;
protected:
	j_value derived_get_value(const Arguments&)const override;
private:
};

j_value GCD_Symbol::derived_get_value(const Arguments& i_args)const {
	if(i_args.empty()){
		return j_value(0, J_Unit());
	}


	j_llint answer = static_cast<j_llint>(i_args[0].value(Arguments()));

	answer = accumulate(i_args.begin() + 1, i_args.end(), answer
		, [](j_llint i_val, j_symbol* i_symbol){
		return i_val + static_cast<j_llint>(i_symbol->value());
	});

	return j_value(static_cast<j_dbl>(answer), J_Unit());
}

GCD_Symbol* GCD_Symbol::get_copy()const {
	return new GCD_Symbol;
}

J_UI_String GCD_Symbol::get_display_name(){
	return "gcd";
}

GCD_Symbol* GCD_Symbol::move_copy(){
	return  new GCD_Symbol(std::move(*this));
}



static void init_reserve_keywords(j_tree<J_UI_String>*);

J_UI_Model& J_UI_Model::get_instance(){
	Instance_Pointer<J_Calc_Data> calc_data_ptr;
	return *calc_data_ptr;
}

J_Calc_Data::J_Calc_Data(){
	init_reserve_keywords(&M_reserved_words);
	init_reserved_symbols();
}

void init_reserve_keywords(j_tree<J_UI_String>* ir_dest_set){
	ir_dest_set->clear();
	ir_dest_set->insert("Dbl");
	ir_dest_set->insert("Alias");
	ir_dest_set->insert("J_Calc");
	ir_dest_set->insert("Int");
	ir_dest_set->insert("Routine");
	ir_dest_set->insert("gcd");
	ir_dest_set->insert("lcm");
	ir_dest_set->insert("least_common_multiple");
}

J_Calc_Data& J_Calc_Data::get_instance(){
	static J_Calc_Data j_calc_data;
	return j_calc_data;
}

void J_Calc_Data::attach_view(J_View_Shared_t i_new_view){
	auto view = dynamic_pointer_cast<J_Calc_View>(i_new_view);
	assert(view);

	J_UI_Model::attach_view(i_new_view);
	bool inserted = M_calc_views.insert(view).second;

	if(!inserted){
		throw J_Argument_Error("View Already A Member of J_Calc_Data");
	}
}


/*void add_user_symbol(J_Symbol_Shared_t)*/
void J_Calc_Data::add_user_symbol(const j_symbol* i_symbol_ptr){
	if(M_user_symbols.count(i_symbol_ptr->name())){
		throw J_Syntax_Error("Symbol With This Name Already Exists");
	}
	M_user_symbols[i_symbol_ptr->name()] = i_symbol_ptr->get_copy();

}

/*j_uint J_Calc_Data::add_math_text_box(a bunch of args)*/
void J_Calc_Data::add_math_text_box(Math_Input_Box_Shared_t i_math_box){

	assert(!M_math_input_boxes.count(i_math_box->get_ID()));
	M_math_input_boxes[i_math_box->get_ID()] = i_math_box;

	add_text_box_object(i_math_box);
}

static Instance_Pointer<J_Calc_Data> s_calc_data;
bool is_reserved_symbol(const J_UI_String& irk_src){
	
	return s_calc_data->is_reserved_symbol(irk_src);
}

j_symbol* get_reserved_symbol(const J_UI_String& irk_string){
	
	return s_calc_data->get_reserved_symbol(irk_string);
}

j_symbol* J_Calc_Data::get_reserved_symbol(const J_UI_String& irk_string)const{
	assert(is_reserved_symbol(irk_string));
	return *M_reserved_symbols.find(irk_string);
}

bool J_Calc_Data::is_reserved_symbol(const J_UI_String& irk_src)const{return M_reserved_symbols.count(irk_src);}
bool J_Calc_Data::is_reserved_word(const J_UI_String& irk_src)const{return M_reserved_words.count(irk_src);}

j_symbol* get_symbol(const J_UI_String& irk_string){
	return s_calc_data->get_symbol(irk_string);
}

j_symbol* J_Calc_Data::get_symbol(const J_UI_String& irk_string)const{
	if(is_reserved_symbol(irk_string)){
		return get_reserved_symbol(irk_string);
	}else if(is_user_symbol(irk_string)){
		return get_user_symbol(irk_string);
	}else{
		throw J_Syntax_Error("No Symbol with that name");
	}
}

bool J_Calc_Data::is_user_symbol(const J_UI_String& irk_string)const{return M_user_symbols.count(irk_string);}

j_symbol* J_Calc_Data::get_user_symbol(const J_UI_String& irk_string)const{
	if(!is_user_symbol(irk_string)){
		throw J_Argument_Error("No User Symbol with that name");
	}

	return *M_user_symbols.find(irk_string);
}

void J_Calc_Data::remove_user_symbol(const J_UI_String& irk_string){
	if(!is_user_symbol(irk_string)){
		throw J_Argument_Error("No User Symbol with that name to remove");
	}

	auto symbol_pos = M_user_symbols.find(irk_string);
	assert(M_user_symbols.end() != symbol_pos);

	M_user_symbols.erase(symbol_pos);
}

/*bool symbol_name_availability_status(const J_UI_String&)*/
bool J_Calc_Data::symbol_name_availability_status(const J_UI_String& irk_string){
	if(is_reserved_word(irk_string)){
		return false;
	}

	if(is_user_symbol(irk_string)){
		return false;
	}

	return true;
}

J_Calc_Data::~J_Calc_Data(){clear_data();}

void J_Calc_Data::clear_data(){
	J_UI_Model::clear_data();

	Calc_J_View_Cont_t().swap(M_calc_views);
	Math_Input_Box_Cont_t().swap(M_math_input_boxes);
	j_tree<J_UI_String>().swap(M_reserved_words);


	for(auto f_reserve_symbol : M_reserved_symbols){
		delete f_reserve_symbol;
	}

	for(auto f_user_symbol : M_user_symbols){
		delete f_user_symbol;
	}

	Symbol_Map_t().swap(M_reserved_symbols);
	Symbol_Map_t().swap(M_user_symbols);
}

void J_Calc_Data::remove_view(J_View_Shared_t i_view){
	J_Calc_View_Shared_t calc_view = dynamic_pointer_cast<J_Calc_View>(i_view);
	assert(calc_view);
	M_calc_views.erase(calc_view);
}

void J_Calc_Data::init_reserved_symbols(){
	M_reserved_symbols["gcd"] = new LLint_Binary_Function_Symbol(greatest_common_divisor_simp
		, "gcd");
	M_reserved_symbols["lcm"] 
		= new LLint_Binary_Function_Symbol(least_common_multiple
		, "least_common_multiple");
}

}