#ifndef J_FUNCTION_H
#define J_FUNCTION_H

#include <J_Fwd_Decl.h>
#include <j_numeric.h>
#include <j_physical_constants.h>
#include "math_base.h"
#include <Data_File.h>
#include "Arguments.h"
#include <J_Error.h>
#include <J_UI\J_UI_String.h>
#include <J_String.h>
#include <ex_array.h>
#include <map>
#include <numeric>
#include <array>
#include <limits>
#include <iostream>
#include <future>
#include <functional>
#include <algorithm>
#include <utility>
#include <ctime>
#include <cassert>
#include <memory>
using namespace std::placeholders;

namespace jomike{


J_FWD_DECL(j_symbol)

J_FWD_DECL(j_value)
J_FWD_DECL(Symbol_Joiner)
J_FWD_DECL(j_composite_symbol)
typedef std::shared_ptr<j_value> J_Value_Shared_t;
typedef std::shared_ptr<j_value> J_Value_Weak_t;

typedef j_symbol_Shared_t J_Symbol_Shared_t;
typedef j_symbol_Weak_t J_Symbol_Weak_t;
typedef j_symbol_Unique_t J_Symbol_Unique_t;

typedef j_composite_symbol_Unique_t J_Composite_Symbol_Unique_t;
static Dbl_t gk_low = g_default_pair.first;
static Dbl_t gk_high = g_default_pair.second;
typedef ex_array<dbl_pair> Bounds_Cont_t;

class j_function{
public:
	virtual j_function* get_copy()const=0;
	virtual ~j_function(){}
	virtual Dbl_t eval(Dbl_t)const = 0;
	Dbl_t operator()(Dbl_t i_x)const{return eval(i_x);}

	//Higher Level Functions
	virtual Dbl_data_t function_param_calc(const Data_t&)const;
	virtual Dbl_data_t function_param_calc_threaded(const Data_t&)const;
	virtual void function_param_calc(Dbl_t*, Dbl_t x, Dbl_t y)const;
	
	virtual Dbl_t func_min()const{return std::numeric_limits<Dbl_t>::lowest();}
	virtual Dbl_t func_max()const{return std::numeric_limits<Dbl_t>::max();}

	virtual Bounds_Cont_t get_def_bounds()const{
		return Bounds_Cont_t(number_of_parameters(), g_default_pair);
	}

	//Altering Functions
	virtual void set(const Dbl_t*)=0;
	virtual void checked_set(const Dbl_t* i_params){set(i_params);}
	virtual void attach(const j_function*){
		throw Unsupported_Function_Call("Cannot Call: attach(const j_function*)");
	}

	//Status Functions
	virtual int number_of_parameters()const=0;
	virtual Dbl_data_t get_params()const =0;
	virtual void set_param();

	//Function Access
	typedef ex_array<j_function*> Function_List_t;
	typedef j_function* const* param_deriv_iterator;
	virtual param_deriv_iterator param_deriv_begin()const{
		//throw Unsupported_Function_Call("Cannot Call: param_deriv_begin()const");
		return nullptr;
	}
	virtual param_deriv_iterator param_deriv_end()const{
		//throw Unsupported_Function_Call("Cannot Call: param_deriv_end()const");
		return nullptr;
	}
private:
	void partial_param_calc(Dbl_t*, const Data_t&, int, int)const;
	
};



class j_composite_function : public j_function{
public:
	j_composite_function(){}
	j_composite_function* get_copy()const override final;
	j_composite_function(const j_composite_function&);
	//Eval

	Dbl_t eval(Dbl_t)const;
//	void function_param_calc(Dbl_t*, Dbl_t x, Dbl_t y)const override final;
	//Altering Functions
	void set(const Dbl_t*)override final;
	void checked_set(const Dbl_t*)throw(Param_Error,Unsupported_Function_Call)override final ;
	void attach(const j_function*)override final;

	//Status Functions
	int size()const{return static_cast<int>(M_functions.size());}
	int number_of_parameters()const override final;
	Dbl_data_t get_params()const override final;
	param_deriv_iterator param_deriv_begin()const override final{
		return M_param_derivs.data();
	}
	param_deriv_iterator param_deriv_end()const override final{
		return M_param_derivs.data()+number_of_parameters();
	}
	~j_composite_function();
private:
	Function_List_t M_functions;
	Function_List_t M_param_derivs;
};

class Wrapper_Function : public j_function{
public:
	Wrapper_Function(const Wrapper_Function& irk_src)
		:M_func(irk_src.M_func->get_copy()){}
	~Wrapper_Function(){delete M_func;}
	Wrapper_Function(const j_function& i_func)
		:M_func(i_func.get_copy()){}
	j_function* get_copy_of_inner()const{return M_func->get_copy();}
	Wrapper_Function():M_func(nullptr){}
	Wrapper_Function& operator=(const Wrapper_Function& irk_src){
		delete M_func;
		M_func = irk_src.get_member()->get_copy();
		return *this;
	}
	//Eval
	Dbl_t eval(Dbl_t i_x)const override{return M_func->eval(i_x);}
	virtual Dbl_data_t function_param_calc(const Data_t& irk_data)const override{
		return M_func->function_param_calc(irk_data);
	}
	void function_param_calc(Dbl_t* i_params, Dbl_t i_x, Dbl_t i_y)const override{
		M_func->function_param_calc(i_params, i_x, i_y);
	}
	param_deriv_iterator param_deriv_begin()const override{
		return M_func->param_deriv_begin();
	}
	param_deriv_iterator param_deriv_end()const override{
		return M_func->param_deriv_end();
	}
	//Status Functions
	int number_of_parameters()const override{return M_func->number_of_parameters();}
	Dbl_data_t get_params()const override{return M_func->get_params();}

	virtual Dbl_t func_min()const{return M_func->func_min();}
	virtual Dbl_t func_max()const{return M_func->func_max();}

	void attach(const j_function* i_func)override{M_func->attach(i_func);}
	//Altering Functions
	void set(const Dbl_t* i_params)override{M_func->set(i_params);}
	void checked_set(const Dbl_t* i_params)override{M_func->checked_set(i_params);}
protected:
	j_function* get_member()const{return M_func;}
	void set_member(j_function* i_func){delete M_func; M_func = i_func;}
private:
	j_function* M_func;
};

class Reliant_Function : virtual public j_function{
public:
	void set(const Dbl_t*)override{
		throw Unsupported_Function_Call("Reliant function cannot call void set(const Dbl_t*)");
	}
	j_function* get_copy()const override final{
		throw Unsupported_Function_Call("Reliant function cannot call j_function* get_copy()const");
	}
	void checked_set(const Dbl_t*)override{
		throw Unsupported_Function_Call("Reliant function cannot call void checked_set(const Dbl_t*)");
	}

private:
};

class Line : public j_function{
public:


	Line(Dbl_t, Dbl_t);
	Line(const Line&);
	Line(){init_derivs();}
	Line* get_copy()const override final{return new Line(*this);}
	Dbl_t eval(Dbl_t i_x)const override final{
		return M_params[0]*i_x + M_params[1];
	}
	int number_of_parameters()const override final{return 2;}
	Dbl_data_t get_params()const{
		return Dbl_data_t(M_params.begin(), M_params.end());
	}
	~Line();
	//Altering Functions
	void set(const Dbl_t*)override final;
	param_deriv_iterator param_deriv_begin()const{return &M_param_funcs.front();}
	param_deriv_iterator param_deriv_end()const{return &M_param_funcs.back()+1;}
	
private:
	std::array<Dbl_t,2> M_params;
	std::array<j_function*, 2> M_param_funcs;
	void init_derivs();
};

typedef Line Line_t;

class Param_Function : public Wrapper_Function{
public:
	Param_Function(){}
	Param_Function* get_copy()const{return new Param_Function(*this);}
	template<typename Iter>
	Param_Function(const j_function& i_func, Iter first, Iter second)
	:Wrapper_Function(i_func), M_bounds(first,second){std::cerr << "\nParam Function Constructed...";}
	Bounds_Cont_t get_def_bounds()const override final{return M_bounds;}
	Param_Function(const j_function& i_func, const Bounds_Cont_t& irk_bounds)
	:Wrapper_Function(i_func), M_bounds(irk_bounds){std::cerr << "\nParam Function Constructed...";}
	Param_Function(const Param_Function& irk_src):Wrapper_Function(*irk_src.get_member())
		,M_bounds(irk_src.M_bounds){std::cerr << "\nParam Function Copied...";}
	//Altering Functions
	void checked_set(const Dbl_t*)throw(Param_Error, Unsupported_Function_Call);
	void set_bounds(const Bounds_Cont_t& new_bounds){M_bounds = new_bounds;}
	const Bounds_Cont_t& get_bounds()const{return M_bounds;}


private:
	Bounds_Cont_t M_bounds;
};

class j_constant : public j_function{
public:
	//Construction
	j_constant();
	j_constant(Dbl_t i_constant);
	j_constant(const j_constant&);
	void set(const Dbl_t* i_param)override final{
		M_constant = *i_param;
	}
	j_constant* get_copy()const override final{return new j_constant(*this);}
	Dbl_t eval(Dbl_t)const override final{return M_constant;}
	int number_of_parameters()const override final{return 1;}
	Dbl_data_t get_params()const override final{return Dbl_data_t(1,M_constant);}

	param_deriv_iterator param_deriv_begin()const{return &M_param_func;}
	param_deriv_iterator param_deriv_end()const{return &M_param_func + 1;}

	~j_constant(){delete M_param_func;}
private:
	Dbl_t M_constant;
	j_function* M_param_func;
};

class j_polynomial_function : public j_function{
public:
	j_polynomial_function* get_copy()const override final{
		return new j_polynomial_function(*this);
	}
	int number_of_parameters()const override final{
		return static_cast<int>(M_params.size());
	}
	j_polynomial_function(int i_degree):M_params(i_degree+1){}
	void set(const Dbl_t*)override final;
	Dbl_t eval(Dbl_t i_x)const override final;
	Dbl_data_t get_params()const override final{
		return Dbl_data_t(M_params.begin(), M_params.end());
	}
private:
	Dbl_data_t M_params;

};

class j_conv : public Wrapper_Function{
public:
	//Construction
	j_conv* get_copy()const override final;
	j_conv(const j_conv&);
	j_conv(const j_function&, const Data_File&);
	j_conv(const j_function&, const Data_File&, Dbl_t step_size);

	//Evals
	Dbl_t eval(Dbl_t)const override final;

	Dbl_data_t function_param_calc(const Data_t& irk_data)const override final{
		return j_function::function_param_calc(irk_data);
	}
	void function_param_calc(Dbl_t*, Dbl_t, Dbl_t)const override final;
	//Altering Functions
	void attach_impulse_data(const Data_File&);

	//Special
	const Data_t& get_data()const{return M_impulse;}
private:
	Data_File M_impulse;
	Dbl_t M_step_size;
	Dbl_t single_slice(j_function*, Dbl_t, int)const;
	Dbl_t single_param_slice(Dbl_t, Dbl_t, int, int)const;
	Dbl_t convolve(j_function*, Dbl_t i_x)const;
	Dbl_t convolve_threaded(j_function*, Dbl_t i_x)const;
	Dbl_t partial_convolve(j_function*, Dbl_t, int start, int end)const;
};

class Interpolation_Evaluation_Method;

enum class Evaluation_Methods{LINEAR, CUBIC_SPLINE};

Evaluation_Methods get_eval_method_enum(const std::string& irk_eval_method);

class Interpolation_Function : public j_function{
public:
	Interpolation_Function(const Data_t& irk_data_file, const std::string& i_eval_method, Dbl_t i_low = gk_low
		, Dbl_t i_high = gk_high){
		Interpolation_Function temp(irk_data_file, get_eval_method_enum(i_eval_method)
			, i_low, i_high);
		swap(temp);
	}
	using j_function::function_param_calc;
	Interpolation_Function(const Data_t& irk_data_file, Evaluation_Methods, Dbl_t i_low = gk_low
		, Dbl_t i_high = gk_high);
	Interpolation_Function(const Interpolation_Function&);
	Dbl_data_t function_param_calc(const Data_t&)const override final;
	Dbl_t eval(Dbl_t)const override;
	Interpolation_Function* get_copy()const override final{return new Interpolation_Function(*this);}
	Interpolation_Function& operator=(const Interpolation_Function& irk_src){
		Interpolation_Function temp(irk_src);
		swap(temp);
		return *this;
	}
	void set(const Dbl_t*)override;
	int number_of_parameters()const override{return 0;}
	Dbl_data_t get_params()const override{return Dbl_data_t();}

	//Special Functions
	dbl_pair range();
	void offset_x_vals(Dbl_t i_offset);

private:
	Interpolation_Evaluation_Method* M_evaluator;
	void swap(Interpolation_Function& ir_src);
};

template<class Func_t>
class j_function_object_wrapper : public j_function{
public:
	j_function_object_wrapper(){}
	j_function_object_wrapper(const Func_t& irk_src):M_func(irk_src){}
	j_function_object_wrapper* get_copy()const override final{
		return new j_function_object_wrapper(*this);
	}
	Dbl_t eval(Dbl_t i_x)const override final{return M_func(i_x);}
private:
	Func_t M_func;
};

template<typename Joiner>
class j_joining_function : public j_function{
public:
	j_joining_function(){}
	j_joining_function* get_copy()const override final;
	j_joining_function(const j_joining_function<Joiner>&);
	//Eval

	Dbl_t eval(Dbl_t)const;

	//Altering Functions
	void set(const Dbl_t*)override final;
	void checked_set(const Dbl_t*)throw(Param_Error,Unsupported_Function_Call)override final ;
	void attach(const j_function*)override final;

	//Status Functions
	int size()const{return static_cast<int>(M_functions.size());}
	int number_of_parameters()const override final;
	Dbl_data_t get_params()const override final;
	param_deriv_iterator param_deriv_begin()const override final{
		return M_param_derivs.data();
	}
	param_deriv_iterator param_deriv_end()const override final{
		return M_param_derivs.data()+number_of_parameters();
	}
	~j_joining_function();
private:
	Function_List_t M_functions;
	Function_List_t M_param_derivs;
	Joiner M_joiner;
};

//SYMBOLIC STOOFS**************************************

struct J_Unit{
	enum class Scalar_Unit{ONE, MOL};
	enum class Time_Unit{SECONDS, MILLISECONDS, YEARS, MINUTES, HOURS};
	enum class Distance_Unit{METERS, CENTIMETERS, MICROMETERS, ANGSTROMS};
	enum class Mass_Unit{GRAMS, KILLIGRAMS, POUNDS, STONES};

	Scalar_Unit M_scalar_unit;
	Time_Unit M_time_unit;
	Distance_Unit M_distance_unit;
	Mass_Unit M_mass_unit;
};



class j_value{
public:
	//Constructors
	j_value(Dbl_t, J_Unit);
	const j_value& operator+(const j_value&)const;

	Dbl_t value()const;

	//Math
	j_value& operator+=(const j_value&);
private:
	J_Unit M_units;
	Dbl_t M_value;
};


class j_symbol{
public:
	j_symbol();
	j_symbol(const J_UI_String&);
	
	//j_symbol(const j_symbol& irk_symbol);
	
	
	//Copy Construction
	virtual j_symbol* get_copy()const =0;

	virtual j_value get_value(const Arguments& i_args = Arguments())const;
	j_dbl value(const Arguments& i_args = Arguments())const;
	virtual void clear() =0;
	virtual bool has_value()const =0;
	const J_UI_String& name()const;
	void set_name(const J_UI_String&);
	virtual J_UI_String get_display_name();
	virtual void set_args(const Arguments&);
	virtual void set_args(Arguments&&);
	virtual ~j_symbol();

	virtual bool is_placeholder()const;
protected:
	virtual j_value derived_get_value(const Arguments& i_args)const =0;
private:
	J_UI_String M_name;
	const int M_ID;
	Arguments M_arguments;
};



class j_composite_symbol : public j_symbol{
public:
	j_composite_symbol(const j_composite_symbol&);
	j_composite_symbol();
	
	bool has_value()const override;
	enum class Join_Type{ADD, MULTIPLY, DIVIDE, SUBTRACT, ERROR_TYPE};
	void append(Join_Type, const j_symbol&);

	j_size_t size()const;

	J_UI_String get_display_name()override;
	j_composite_symbol* get_copy()const override;

	j_symbol& operator[](j_size_t i_index);

	const j_symbol& operator[](j_size_t i_index)const;

	//Destruction
	~j_composite_symbol();
	void clear()override;
protected:
	j_value derived_get_value(const Arguments&)const override;
private:
	ex_array<Symbol_Joiner*> M_symbol_joiners;
	ex_array<j_symbol*> M_symbols;

};


j_composite_symbol::Join_Type get_composite_symbol_join_type(J_UI_Char i_char);

class J_Sym_Argument_Error: public J_Error{
public:
	J_Sym_Argument_Error(const char * const ik_message = "invalid args");
private:
};

class J_Value_Error: public J_Error{
public:
	J_Value_Error(const char * const ik_message = "NO Value in symbol");
private:
};


//class j_number_symbol **************************
class j_number_symbol : public j_symbol{
public:
	//Constructors
	j_number_symbol(Dbl_t);
	j_number_symbol(const J_UI_String&);

	j_number_symbol* get_copy()const override;

	//Status
	bool has_value()const override;



	J_UI_String get_display_name()override;

	//Reset
	void clear()override;
protected:
	j_value derived_get_value(const Arguments&)const override;
private:
	bool M_value_status;
	Dbl_t M_value;
};

//To Do: Define these functions
j_symbol* get_symbol(const J_UI_String&);

j_symbol* get_reserved_symbol(const J_UI_String&);

class j_symbolic_function : public j_function{
public:
	j_symbolic_function(const J_UI_String&);
	j_symbolic_function* get_copy()const override;

	const std::string& name();
	void set_name(const std::string&);
protected:

private:
	std::string M_name;
	std::vector<J_Symbol_Weak_t> M_symbols_by_name;
	
};

class j_placeholder_symbol : public j_symbol{
public:
	j_placeholder_symbol* get_copy()const override{
		return new j_placeholder_symbol(*this);
	}
	bool has_value()const override;
	void clear()override;
	j_placeholder_symbol(j_size_t i_index);
	bool is_placeholder()const override;
	j_size_t placeholder_index()const;
protected:
	j_value derived_get_value(const Arguments& i_args)const override;
private:
	j_size_t M_Placeholder_index;
};

class J_Routine_Wrapper_Symbol : public j_symbol{
public:
	void clear()override;
	bool has_value()const override;
private:
};

J_FWD_DECL(J_Routine_Symbol)

class J_Routine_Symbol : public j_symbol{
public:

	J_Routine_Symbol* get_copy()const override;

	bool symbol_name_availability_status(const J_UI_String&);
	j_symbol* get_scope_symbol(const J_UI_String&);
	void add_scope_symbol(const j_symbol&);
	void clear()override;
	bool has_value()const override;
protected:
private:
	typedef std::map<J_UI_String, j_symbol*> Symbol_Cont_t;
	Symbol_Cont_t M_scope_symbols;
	ex_array<std::string> M_statements;
	j_value derived_get_value(const Arguments&)const override;
};

class GCD_Symbol : public J_Routine_Wrapper_Symbol{
public:
	GCD_Symbol* get_copy()const override;
	J_UI_String get_display_name()override;
protected:
	j_value derived_get_value(const Arguments&)const override;
private:
};

template<typename Binary_Function_t, typename Num_t = j_dbl>
class Binary_Function_Chain_Symbol : public J_Routine_Wrapper_Symbol{
public:
	Binary_Function_Chain_Symbol(const Binary_Function_t&, const std::string&);
	Binary_Function_Chain_Symbol* get_copy()const override;
	J_UI_String get_display_name()override;
protected:
	j_value derived_get_value(const Arguments&)const override;
private:
	Binary_Function_t M_function;
	J_UI_String M_name;
};

typedef
Binary_Function_Chain_Symbol<j_llint(*)(j_llint, j_llint), j_llint>
	LLint_Binary_Function_Symbol;

template<typename Binary_Function_t, typename Num_t /*= j_dbl*/>
j_value Binary_Function_Chain_Symbol<Binary_Function_t, Num_t>
	::derived_get_value(const Arguments& irk_args)const{
	if(irk_args.empty()){
		return j_value(0, J_Unit());
	}


	Num_t answer = static_cast<j_llint>(irk_args[0].value(Arguments()));

	answer = std::accumulate(irk_args.begin() + 1, irk_args.end(), answer
		, [&](Num_t i_val, j_symbol* i_symbol){
		return M_function(i_val, static_cast<Num_t>(i_symbol->value()));
	});

	return j_value(static_cast<j_dbl>(answer), J_Unit());
}


template<typename Binary_Function_t, typename Num_t /*= j_dbl*/>
Binary_Function_Chain_Symbol<Binary_Function_t, Num_t>* 
	Binary_Function_Chain_Symbol<Binary_Function_t, Num_t>::get_copy()const {

	return new Binary_Function_Chain_Symbol<Binary_Function_t, Num_t>(*this);
}


template<typename Binary_Function_t, typename Num_t /*= j_dbl*/>
J_UI_String Binary_Function_Chain_Symbol<Binary_Function_t, Num_t>::get_display_name(){
	return M_name;
}


template<typename Binary_Function_t, typename Num_t /*= j_dbl*/>
Binary_Function_Chain_Symbol<Binary_Function_t, Num_t>
::Binary_Function_Chain_Symbol(const Binary_Function_t& irk_function
	, const std::string& irk_name):M_function(irk_function), M_name(irk_name){
	set_args(Arguments(2));
}


template<typename Joiner>
j_joining_function<Joiner>* j_joining_function<Joiner>::get_copy()const{
	return new j_joining_function<Joiner>(*this);
}

template<typename Joiner>
void j_joining_function<Joiner>::set(const Dbl_t* i_params){
	for(int i=0; i < size();  i++){
		M_functions[i]->set(i_params);
		i_params += (M_functions[i]->number_of_parameters());
	}
}
template<typename Joiner>
void j_joining_function<Joiner>::checked_set(const Dbl_t* i_params) 
	throw(Param_Error, Unsupported_Function_Call){
	for(int i=0; i < size(); i++){
		M_functions[i]->checked_set(i_params);
		i_params += (M_functions[i]->number_of_parameters());
	}
}
template<typename Joiner>
int j_joining_function<Joiner>::number_of_parameters()const{
	return std::accumulate(M_functions.begin(), M_functions.end(), int(0)
		,[](int first, const j_function* i_func){return first+=i_func->number_of_parameters();});
}
template<typename Joiner>
j_joining_function<Joiner>::~j_joining_function(){
	for_each(M_functions.begin(), M_functions.end(),[](j_function* i_func){delete i_func;});
}
template<typename Joiner>
Dbl_data_t j_joining_function<Joiner>::get_params()const{
	Dbl_data_t params;
	auto inserter = back_inserter(params);
	for(int i=0; i < size(); i++){
		Dbl_data_t temp_params = M_functions[i]->get_params();
		copy(temp_params.begin(), temp_params.end(), inserter);
	}
	return params;
}
template<typename Joiner>
j_joining_function<Joiner>::j_joining_function(const j_joining_function<Joiner>& irk_source){
	for_each(irk_source.M_functions.begin(), irk_source.M_functions.end()
		,std::bind(&j_joining_function<Joiner>::attach, this, _1));
	std::cerr << "\nJoiner Function Copied...";
}

template<typename Joiner>
void j_joining_function<Joiner>::attach(const j_function* i_new_func){
	auto param_deriv_it = back_inserter(M_param_derivs);
	M_functions.push_back(i_new_func->get_copy());
	copy(M_functions.back()->param_deriv_begin(), M_functions.back()->param_deriv_end()
		, param_deriv_it);
}

template<typename Joiner>
Dbl_t j_joining_function<Joiner>::eval(Dbl_t i_x)const{
	assert(size());
	auto func_iterator = M_functions.begin();
	Dbl_t result =  (*func_iterator++)->eval(i_x);
	while(func_iterator < M_functions.end()){
		result = M_joiner(result, (*func_iterator++)->eval(i_x));
	}

	return result;
}

bool is_char_allowed(J_UI_Char i_char);

}

#endif