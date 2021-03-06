#include "If_Statement.h"
//
#include "j_calc_expression.h"
//
#include "Statement_Block.h"
//
#include <j_symbol/Specific_Symbol_List.h>
//
#include "Expression_Statement.h"
namespace jomike{


If_Statement::If_Statement(const yy::location& irk_loc
	, j_calc_expression* i_test_expression, Statement_Block* i_then_body
	, j_statement* i_else_body /*= nullptr*/): j_statement(irk_loc, Symbol_Types::IF_STATEMENT){
	M_test_expression = i_test_expression;
	M_then_body = i_then_body;
	M_else_body = i_else_body;
}

If_Statement* If_Statement::move_copy(){
	return  new If_Statement(std::move(*this));
}

If_Statement* If_Statement::get_copy()const {
	return new If_Statement(*this);
}

If_Statement::If_Statement(const If_Statement& irk_source):j_statement(irk_source){
	assert(irk_source.M_test_expression);
	assert(irk_source.M_then_body);

	M_test_expression = irk_source.M_test_expression->get_copy();
	M_then_body = irk_source.M_then_body->get_copy();
	if(irk_source.M_else_body){
		M_else_body = irk_source.M_else_body->get_copy();
	}
}


If_Statement::If_Statement(If_Statement&& irv_source):j_statement(std::move(irv_source)){
	assert(irv_source.M_test_expression);
	assert(irv_source.M_then_body);

	M_test_expression = irv_source.M_test_expression->move_copy();
	M_then_body = irv_source.M_then_body->move_copy();
	
	M_else_body = irv_source.M_else_body;
	irv_source.M_else_body = nullptr;
}



void If_Statement::clear(){
	M_test_expression->clear();
	delete M_else_body;
	M_else_body = nullptr;
	M_then_body->clear();
}

bool If_Statement::has_value()const {
	if(!M_test_expression->has_value()){
		return false;
	}

	if(M_test_expression->get_value().as_bool()){
		return M_then_body->has_value();

	} else{
		return M_else_body ? M_else_body->has_value() : false;
	}
}

If_Statement::~If_Statement(){
	delete M_test_expression;
	delete M_then_body;
	delete M_else_body;
}

j_value If_Statement::derived_get_value(const Arguments& i_args)const {
	if(M_test_expression->get_value().as_bool()){
		return M_then_body->get_value(i_args);
	} else if(M_else_body){
		return M_else_body->get_value(i_args);
	} else{
		return j_value();
	}
}




void If_Statement::alert_symbol_scope_set(){
	assert(M_test_expression);
	assert(M_then_body);

	M_test_expression->set_symbol_scope(&symbol_scope());
	M_then_body->set_symbol_scope(&symbol_scope());
	if(M_else_body){
		M_else_body->set_symbol_scope(&symbol_scope());
	}
}

void If_Statement::process(){
	assert(M_test_expression);
	assert(M_then_body);
	M_test_expression->process();
	M_then_body->process();
	if(M_else_body){
		M_else_body->process();
	}
}

}



