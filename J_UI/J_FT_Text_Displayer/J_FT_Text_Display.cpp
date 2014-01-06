#include "J_FT_Text_Display.h"
//
#include "../J_Image/J_Image.h"
//
#include <J_OpenGL.h>
//
#include <J_Shader_Program.h>
//
#include <J_Utile.h>
//
#include "../J_UI_Controller.h"
//
#include "../j_cursor_management.h"

//
#include "../J_Font_Manager.h"

//Algorithms
#include <algorithm>
#include <functional>
#include <numeric>
using std::bind; using std::copy; using std::equal_to; using std::for_each; using std::is_sorted;
using std::max; using std::min; using std::none_of; using std::accumulate;
using namespace std::placeholders;using std::max_element;

//Containers
#include <ex_array.h>

using std::array; using std::string;

//IO Facilities

#include <thread>
using std::thread;

#include <iostream>
using std::cerr; using std::endl;

//Libraries

//
#include <gl\glew.h>

//Utilities
#include <cassert>
using std::mutex;
using std::pair;




using std::lock_guard;


namespace jomike{
static Instance_Pointer<Contexts_Handler> s_contexts;

const j_float FLOAT_DELTA = 0.0001f;
const j_float K_LEFT_PADDING = 0.01f;

const char* const DEFAULT_FONT_PATH_NAME 
	= "J:\\JoMike Library\\J_FT_Text_Display\\Fonts\\times.ttf";


class J_Text_Shader_Program{
public:
	J_Text_Shader_Program();
	int program_id(){return M_program_id;};
	int cursor_program_id(){return M_cursor_program_id;}
private:
	
	int M_program_id;
	j_uint M_cursor_program_id;
};

J_Text_Shader_Program::J_Text_Shader_Program(){
	M_program_id = glCreateProgram();
	
	j_uint vert_shader_id = load_vertex_shader("Shaders/quad.vert");
	j_uint frag_shader_id = load_fragment_shader("Shaders/quad_texture.frag");

	glAttachShader(M_program_id, vert_shader_id);
	glAttachShader(M_program_id, frag_shader_id);
	glLinkProgram(M_program_id);

	enforce_program_status(M_program_id, GL_LINK_STATUS);

	M_cursor_program_id = glCreateProgram();
	j_uint outline_shader_id = load_vertex_shader("Shaders/quad.vert");
	j_uint outline_frag_shader_id = load_fragment_shader("Shaders/box_outline.frag");

	glAttachShader(M_cursor_program_id, outline_shader_id);
	glAttachShader(M_cursor_program_id, outline_frag_shader_id);
	glLinkProgram(M_cursor_program_id);

	//cerr << "\n Shader Program ID: " << M_program_id << "*********************************************************";
}




class Text_Line_Order_Comp{
public:
	Text_Line_Order_Comp(j_float i_start_line, j_float i_line_size):M_line_size(i_line_size)
		, M_starting_pos(i_start_line){}

	bool operator()(const Pen_Pos_FL_t, const Pen_Pos_FL_t)const;
private:
	int get_line_index(j_float i_float)const;
	j_float M_line_size;
	j_float M_starting_pos;

};

int Text_Line_Order_Comp::get_line_index(j_float i_float)const{
	int line_number = 0;

	while( i_float < (M_starting_pos - line_number*M_line_size) - FLOAT_DELTA){
		++line_number;
	}

	if(line_number){
		return line_number;
	}

	if(i_float < M_starting_pos){
		return line_number;
	}

	while(i_float >(M_starting_pos - line_number*M_line_size) - FLOAT_DELTA){
		--line_number;
	}

	return line_number;
}

bool Text_Line_Order_Comp::operator()(const Pen_Pos_FL_t ik_left, const Pen_Pos_FL_t ik_right)const{
	int compare_val = get_line_index(ik_right.second) - get_line_index(ik_left.second);
	if(compare_val < 0){
		return false;
	}else if(compare_val > 0){
		return true;
	}else if((ik_right.first - ik_left.first) > 0){
		return true;
	}else{
		return false;
	}
}


J_FT_Text_Display_Object::J_FT_Text_Display_Object(j_uint i_obj_id)
	:J_Display_Box(i_obj_id){}

J_FT_Text_Display_Object::J_FT_Text_Display_Object(j_uint i_object_id, j_uint i_id)
	:J_Display_Box(i_object_id, i_id){}

J_FT_Text_Display_Object::~J_FT_Text_Display_Object(){}


J_FT_Text_Display::J_FT_Text_Display(j_uint i_obj_id): J_FT_Text_Display_Object(i_obj_id){
	M_new_line_size = 30;
	M_shader = new J_Text_Shader_Program;
	M_program_id = M_shader->program_id();
	M_pen_poses.push_back(default_pen_pos());
	M_pen_poses.set_front_buffer(0);
	init_cursor_vao();
}

J_FT_Text_Display::J_FT_Text_Display(j_uint i_object_id, j_uint i_id)
	:J_FT_Text_Display_Object(i_object_id, i_id){
	M_new_line_size = 30;
	M_shader = new J_Text_Shader_Program;
	M_program_id = M_shader->program_id();
	M_pen_poses.push_back(default_pen_pos());
	M_pen_poses.set_front_buffer(0);
	init_cursor_vao();
}



void J_FT_Text_Display::recalculate_text_positions(){
	j_size_t cursor_pos = M_cursor_pos;
	J_UI_Multi_String cur_string(M_string);
	set_string(cur_string);
	set_cursor_pos_no_scroll(cursor_pos);
}

void J_FT_Text_Display::init_cursor_vao(){
	glGenVertexArrays(1, &M_cursor_vao_id);
	glBindVertexArray(M_cursor_vao_id);

	glGenBuffers(1, &M_cursor_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, M_cursor_buffer_id);
	std::array<j_float, 4> cursor_data;
	glBufferData(GL_ARRAY_BUFFER, sizeof(cursor_data), cursor_data.data(), GL_DYNAMIC_DRAW); 
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, nullptr);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	set_cursor_pos(0);
}

void J_FT_Text_Display::set_cursor_pos(j_size_t i_pos){
	assert(between_inclusive(i_pos, J_SIZE_T_ZERO, M_pen_poses.size() - 1));
	M_last_set_cursor_pos = M_pen_poses[i_pos];
	auto_scroll_window(i_pos);

	set_cursor_pos_no_scroll(i_pos);

}

void J_FT_Text_Display::auto_scroll_window(j_size_t i_pos){
	if(!auto_scrolling_status()){
		return;
	}
	if(is_cursor_pos_in_view(i_pos)){
		return;
	}

	auto pen_pos_y_lower = M_pen_poses[i_pos].second;
	auto pen_pos_y_upper = M_pen_poses[i_pos].second + new_line_screen_size();

	Pen_Pos_FL_t new_starting_pen_pos = M_pen_poses.front();
	if(pen_pos_y_upper > y1()){
		new_starting_pen_pos.second -= (pen_pos_y_upper - y1() + FLOAT_DELTA);
		new_starting_pen_pos.second 
			= max(new_starting_pen_pos.second, default_pen_pos().second);
	} else{
		assert(pen_pos_y_lower < y2());
		new_starting_pen_pos.second += (y2() - pen_pos_y_lower + FLOAT_DELTA);
	}
	scroll_selection_boxes(0.0
						   ,  new_starting_pen_pos.second - M_pen_poses.front().second);
	set_starting_pen_pos(new_starting_pen_pos);
}



void J_FT_Text_Display::set_starting_pen_pos(Pen_Pos_FL_t i_pen_pos){
	auto cursor_pos = M_cursor_pos;
	auto cur_string = M_string;
	clear();
	M_pen_poses.front() = i_pen_pos;
	
	
	M_new_line_size = cur_string.empty() ? M_new_line_size
		: max_element(cur_string.begin(), cur_string.end()
		, [](const J_UI_String& yrk_left, const J_UI_String& yrk_right){
		return (yrk_left.font_face()->new_line_size
			< yrk_right.font_face()->new_line_size);
	})->font_face()->new_line_size;


	M_new_line_size = static_cast<int>(M_new_line_size*0.70);

	for(const J_UI_String& f_string : cur_string){
		add_string(f_string);
	}
	set_cursor_pos_no_scroll(cursor_pos);
			
}

void J_FT_Text_Display::set_cursor_color(J_Color_RGBA<j_float> i_color){
	int uniform_loc = glGetUniformLocation(M_shader->cursor_program_id(), "u_outline_color");
	assert(-1 != uniform_loc);
	glProgramUniform4fv(M_shader->cursor_program_id(), uniform_loc, 1, i_color.M_data.data());
}

void J_FT_Text_Display::draw_cursor()const{
	if(!is_cursor_pos_in_view(M_cursor_pos)){
		return;
	}
	glUseProgram(M_shader->cursor_program_id());
	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(M_cursor_vao_id);
	glLineWidth(1.0f);
	
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
	glUseProgram(0);
}

Pen_Pos_FL_t J_FT_Text_Display::default_pen_pos()const{
	return Pen_Pos_FL_t(x1()+K_LEFT_PADDING, y1() 
		- to_y_screen(s_contexts->get_active_window(),M_new_line_size));
}

void J_FT_Text_Display::set_string(const J_UI_Multi_String& irk_string){
	
	M_new_line_size = irk_string.empty() ? M_new_line_size 
		: max_element(irk_string.begin(), irk_string.end()
		,[](const J_UI_String& yrk_left, const J_UI_String& yrk_right){
			return (yrk_left.font_face()->new_line_size 
				< yrk_right.font_face()->new_line_size);	
		})->font_face()->new_line_size;

	clear();
	M_new_line_size = static_cast<int>(M_new_line_size*0.90);

	for(const J_UI_String& f_string: irk_string){
		add_string(f_string);
	}

}

void J_FT_Text_Display::clear(){
	M_pen_poses.front() = default_pen_pos();
	clear_from(0);
}

void J_FT_Text_Display::clear_from(j_size_t i_pos){
	assert(i_pos >= 0);
	//delete_vaos_from(i_pos);
	//delete_buffers_from(i_pos);
	M_pen_poses.resize(i_pos + 1);
	
	M_string.resize(i_pos);
	if(M_cursor_pos > i_pos){
		set_cursor_pos_no_scroll(i_pos);
	}
}


/*void J_FT_Text_Display::draw()const*/
void J_FT_Text_Display::draw()const{
	J_Display_Box::draw();
	for(auto f_box : M_selection_boxes){
		f_box->draw();
	}


	j_float y_size = new_line_screen_size();

	auto start_view_pos = lower_bound(M_pen_poses.begin()
		, M_pen_poses.end(), Pen_Pos_FL_t(x1(), default_pen_pos().second)
		, Text_Line_Order_Comp(default_pen_pos().second
		, y_size));

	auto end_view_pos = lower_bound(M_pen_poses.begin()
		, M_pen_poses.end(), Pen_Pos_FL_t(x2(), y2())
		, Text_Line_Order_Comp(y1(), y_size));

	j_size_t begin_pos = start_view_pos - M_pen_poses.begin();
	j_size_t end_pos = end_view_pos - M_pen_poses.begin();
	--end_pos;
	for(j_size_t i = begin_pos; i < end_pos; ++i){
		draw_pos(i);
	}
	if(cursor_visibility_status()){
		draw_cursor();
	}
}
void J_FT_Text_Display::draw_string(J_UI_Multi_String::const_iterator irk_string_pos)const{
	glUseProgram(M_program_id);
	int uniform_loc = glGetUniformLocation(M_program_id, "tex_color");
	assert(-1 != uniform_loc);
	glProgramUniform4fv(M_program_id, uniform_loc, 1, irk_string_pos->color().M_data.data());

	

	j_size_t start_pos = accumulate(M_string.cbegin(), irk_string_pos, J_SIZE_T_ZERO
		, [](j_size_t val , const J_UI_String& yrk_string){
			return val + yrk_string.size();
		});

	j_size_t end_pos = start_pos + irk_string_pos->size();

	for(j_size_t i=start_pos; i < end_pos; i++){
		switch(M_string[i].charcode()){
		case '\n':
			break;
		default:
			 draw_to_buffer(i);
		}
	}
	
	glUseProgram(0);
}

void J_FT_Text_Display::draw_pos(j_size_t i_pos)const{
	auto string_pos = M_string.get_string_holding_index(i_pos);
	glUseProgram(M_program_id);
	int uniform_loc = glGetUniformLocation(M_program_id, "tex_color");
	assert(-1 != uniform_loc);
	glProgramUniform4fv(M_program_id, uniform_loc, 1, string_pos->color().M_data.data());


	switch(M_string[i_pos].charcode()){
	case '\n':
		break;
	default:
		draw_to_buffer(i_pos);
	}


	glUseProgram(0);
}


void J_FT_Text_Display::alert_resize(int,int){
	recalculate_text_positions();
}

void J_FT_Text_Display::draw_to_buffer(j_size_t i_index)const{

	
	//cerr << "\nBit Map WIDTH: " << expanded_bitmap.width() << " Bit Map Rows: " << expanded_bitmap.height() <<
	//	" Pitch: " << glyph->bitmap.pitch << endl;

	glBindVertexArray(M_vao_ids[i_index]);

	glBindTexture(GL_TEXTURE_2D, M_string.texture_id_at_index(i_index));

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	//cerr << "\nx_resolution: " << x_resolution << " y_resolution: " << y_resolution << " y_ppem: " 
	//	<< M_curface->size->metrics.y_ppem;
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void J_FT_Text_Display::insert_char(j_size_t i_pos, J_UI_Char i_char){
	J_UI_Multi_String string_remaining = M_string.substr(i_pos);

	clear_from(i_pos);
	add_char(i_char);
	++M_cursor_pos;
	add_multi_string(string_remaining);
	set_cursor_pos(i_pos+1);
}

void J_FT_Text_Display::delete_char(j_size_t i_pos){
	assert(i_pos >= 0);
	assert( i_pos < M_string.size());

	J_UI_Multi_String string_remaining = M_string.substr(i_pos + 1);

	clear_from(i_pos);
	--M_cursor_pos;
	add_multi_string(string_remaining);
	set_cursor_pos(i_pos);
}

void J_FT_Text_Display::erase_chars(j_size_t i_pos, j_size_t i_size){
	assert(i_pos >= 0);
	assert(i_size >= 0);
	assert( i_pos < M_string.size());
	assert(i_pos + i_size <= M_string.size());
	J_UI_Multi_String string_remaining = M_string.substr(i_pos + i_size);

	clear_from(i_pos);

	add_multi_string(string_remaining);

	
	set_cursor_pos(i_pos);
}

void J_FT_Text_Display::add_multi_string(const J_UI_Multi_String& irk_string){
	for_each(irk_string.begin(), irk_string.end()
	, bind(&J_FT_Text_Display::add_string, this, _1));
}

void J_FT_Text_Display::add_string(const J_UI_String& irk_string){
	if(M_string.empty() || !M_string.back().has_same_font_and_color(irk_string)){
		M_string.push_back(J_UI_String(irk_string.font_face(), irk_string.color()));
	}
	for(auto f_char : irk_string){
		add_char(f_char);
	}

	set_cursor_pos_no_scroll(M_cursor_pos + static_cast<int>(irk_string.size()));
}

void J_FT_Text_Display::insert_chars(j_size_t i_pos, const J_UI_String& irk_string){
	J_UI_Multi_String string_remaining = M_string.substr(i_pos);
	clear_from(i_pos);

	for(j_size_t i=0; i < irk_string.size(); i++){
		add_char(irk_string[i]);
	}

	set_cursor_pos(M_cursor_pos + irk_string.size());
}

void J_FT_Text_Display::add_char(J_UI_Char i_char){
	//assert(get_context() == s_contexts->get_active_context());
	if(M_vao_ids.size() <= M_string.size()){
		add_new_vao_and_vbo(safe_int_cast(2*(M_vao_ids.size()+1)));
	}

	auto& bitmap_metric = M_string.back().font_face()->M_bitmap_metrics[i_char.charcode()];

	if('\n' == i_char.charcode()){
		M_string.push_back(i_char);
		M_pen_poses.push_back(new_line_pen_pos(M_pen_poses.back()));
		return;
	}
	auto window = s_contexts->get_active_window();
	
	j_float width = to_x_screen(window, bitmap_metric.width);
	j_float height = to_y_screen(window,bitmap_metric.height);
	
	j_float x_start, y_start;

	

	if((M_pen_poses.back().first+width) > x2()){
		Pen_Pos_FL_t new_pos(new_line_pen_pos(M_pen_poses.back()));
		x_start = new_pos.first;
		y_start = new_pos.second;
		M_pen_poses.push_back(calculate_pen_advance(new_pos
		,bitmap_metric.advance_x));
	}else{
		x_start = M_pen_poses.back().first;
		y_start = M_pen_poses.back().second;
		M_pen_poses.push_back(calculate_pen_advance(M_pen_poses.back()
		, bitmap_metric.advance_x));
	}
	j_float x_screen_pos = x_start
		+ to_x_screen(window, bitmap_metric.left);

	j_float y_screen_pos = y_start 
		- to_y_screen(window, bitmap_metric.underreach);


	//cerr << "\nx_screen_pos: " << x_screen_pos << " y_screen_pos: " << y_screen_pos 
	//	<< " Text Width: " << width << " Text Height: " << height;

	array<j_float,24> vertex_array_data = {
		x_screen_pos, y_screen_pos, 0.0f, 1.0f
		,x_screen_pos +  width, y_screen_pos, 0.0f, 1.0f
		, x_screen_pos + width, y_screen_pos + height, 0.0f, 1.0f
		,x_screen_pos, y_screen_pos + height, 0.0, 1.0f

		,0.0f, 1.0f
		,1.0f, 1.0f
		,1.0f, 0.0f
		,0.0f, 0.0f
	};
	j_size_t index = M_string.size();
	M_string.push_back(i_char.charcode());
	
	glBindVertexArray(M_vao_ids[index]);
	glBindBuffer(GL_ARRAY_BUFFER, M_vao_buffer_ids[index]);
	
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_array_data)
		, vertex_array_data.data());

	glBindVertexArray(0);

}

void J_FT_Text_Display::add_new_vao_and_vbo(int i_num_new_vaos){
	j_size_t old_num_vaos = M_vao_ids.size();
	M_vao_ids.resize(old_num_vaos + i_num_new_vaos);
	M_vao_buffer_ids.resize(old_num_vaos + i_num_new_vaos);

	glGenVertexArrays(i_num_new_vaos, &M_vao_ids[old_num_vaos]);
	
	glGenBuffers(i_num_new_vaos, &M_vao_buffer_ids[old_num_vaos]);
	
	//static ex_array<j_float> dummy(24, 0.0f);

	for(j_size_t i = old_num_vaos; i < M_vao_ids.size(); i++){
	
		glBindVertexArray(M_vao_ids[i]);
		glBindBuffer(GL_ARRAY_BUFFER, M_vao_buffer_ids[i]);
		glBufferData(GL_ARRAY_BUFFER, 24*sizeof(j_float), nullptr, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, nullptr);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, reinterpret_cast<void*>(16*sizeof(GLfloat)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}
	glBindVertexArray(0);
}

/*Pen_Pos_t calculate_pen_advance(const J_UI_String_Data*,Pen_Pos_t, int)*/
Pen_Pos_FL_t J_FT_Text_Display::calculate_pen_advance(Pen_Pos_FL_t i_cur_pen, int i_advance_val_pixels)const{
	i_cur_pen.first += to_x_screen(s_contexts->get_active_window(), i_advance_val_pixels);

	return i_cur_pen;
}

/*Pen_Pos_t calculate_pen_advance(const J_UI_String_Data*,Pen_Pos_t, int)*/
Pen_Pos_FL_t J_FT_Text_Display::new_line_pen_pos(Pen_Pos_FL_t i_pen)const{
	i_pen.first = x1() + K_LEFT_PADDING;
	i_pen.second -= to_y_screen(s_contexts->get_active_window(), M_new_line_size);
	return i_pen;
}

J_FT_Text_Display::~J_FT_Text_Display(){
	delete_vaos();
	delete_buffers();
	delete M_shader;
}

void J_FT_Text_Display::delete_vaos(){
	delete_vaos_from(0);
}

void J_FT_Text_Display::delete_vaos_from(j_size_t i_pos){
	//assert(get_context() == s_contexts->get_active_context());
	assert(M_vao_ids.size() >= i_pos);
	
	if(M_vao_ids.size() == i_pos){
		return;
	}

	glDeleteVertexArrays(static_cast<int>(M_vao_ids.size() - i_pos), &M_vao_ids[i_pos]);
	M_vao_ids.resize(i_pos);
}

void J_FT_Text_Display::delete_buffers(){
	delete_buffers_from(0);
}

void J_FT_Text_Display::delete_buffers_from(j_size_t i_pos){
	//assert(get_context() == s_contexts->get_active_context());
	assert(M_vao_buffer_ids.size() >= i_pos);
	if(M_vao_buffer_ids.size() == i_pos){
		return;
	}

	glDeleteBuffers(static_cast<int>(M_vao_buffer_ids.size() - i_pos), &M_vao_buffer_ids[i_pos]);
	M_vao_buffer_ids.resize(i_pos);
}

void J_FT_Text_Display::mouse_button_press(J_View_Shared_t i_view, int i_button, int , Pen_Pos_FL_t i_pos){
	
	switch(i_button){
	case J_LEFT_MOUSE_BUTTON:{
		M_selection_boxes.clear();
		M_selection_start_cursor_pos = get_cursor_index(i_pos);
		M_left_mouse_button_pressed_status = true;
		set_cursor_pos(M_selection_start_cursor_pos);
		J_UI_Controller::get_instance().notify_text_box_press(i_view, get_object_ID()
															, M_cursor_pos);
}
		break;
	case J_MOUSE_WHEEL_UP:
		scroll(lines_scrolled_per_tick());
		break;
	case J_MOUSE_WHEEL_DOWN:
		scroll(-lines_scrolled_per_tick());
		break;
	default:
		;
	}
}

void J_FT_Text_Display::mouse_button_release(J_View_Shared_t, int i_button, int , Pen_Pos_FL_t i_pos){

	switch(i_button){
	case J_LEFT_MOUSE_BUTTON:
		M_left_mouse_button_pressed_status = false;
		set_cursor_pos(get_cursor_index(i_pos));
		J_UI_Controller::get_instance().notify_text_box_release(get_object_ID()
			, get_cursor_index(i_pos));
		break;
	default:
		;
	}
}



/*int get_cursor_index(Pen_Pos_FL_t)const*/
j_size_t J_FT_Text_Display::get_cursor_index(Pen_Pos_FL_t i_pen_pos)const{
	j_float line_screen_size = to_y_screen(s_contexts->get_active_window(), M_new_line_size);
	
	Text_Line_Order_Comp text_pos_compare_fo(M_pen_poses.front().second
		+ line_screen_size, line_screen_size);

	assert(is_sorted(M_pen_poses.begin(), M_pen_poses.end()
		, text_pos_compare_fo));

	ptrdiff_t pos = lower_bound(M_pen_poses.begin(), M_pen_poses.end(), i_pen_pos
				, text_pos_compare_fo)
			- M_pen_poses.begin();

	j_size_t string_size = M_string.size();
	return max(min(pos - 1, string_size), J_SIZE_T_ZERO);
}

void J_FT_Text_Display::pop_back(){

	assert(!"The method or operation is not yet check.");
}




bool J_FT_Text_Display::cursor_visibility_status()const{
	return M_cursor_visibility_status;
}

void J_FT_Text_Display::set_cursor_visibility_status(bool i_status){
	M_cursor_visibility_status = i_status;
}

bool J_FT_Text_Display::auto_scrolling_status()const{
	return M_auto_scrolling_status;
}

void J_FT_Text_Display::set_auto_scrolling_status(bool i_status){
	M_auto_scrolling_status = i_status;
}

bool J_FT_Text_Display::is_cursor_pos_in_view(j_size_t i_pos)const {
	auto y_bottom_pos = M_pen_poses[i_pos].second;
	auto y_top_pos = y_bottom_pos + new_line_screen_size();
	return is_y_inside(y_bottom_pos) && is_y_inside(y_top_pos);
}

j_float J_FT_Text_Display::new_line_screen_size()const{
	return to_y_screen(s_contexts->get_active_window(), M_new_line_size);
}

void J_FT_Text_Display::move_cursor_line_pos_up(j_size_t i_move_val){
	auto new_pen_pos = M_last_set_cursor_pos;
	new_pen_pos.second += i_move_val*new_line_screen_size();
	set_cursor_pos(get_cursor_index(new_pen_pos));
	M_last_set_cursor_pos = M_pen_poses[M_cursor_pos];
	M_last_set_cursor_pos.first = new_pen_pos.first;

	J_UI_Controller::get_instance().notify_text_box_press(J_View_Shared_t(), get_object_ID()
														  , M_cursor_pos);
}

void J_FT_Text_Display::move_cursor_line_pos_down(j_size_t i_move_val){
	auto new_pen_pos = M_last_set_cursor_pos;
	new_pen_pos.second -= (i_move_val*new_line_screen_size() + FLOAT_DELTA);
	set_cursor_pos(get_cursor_index(new_pen_pos));
	M_last_set_cursor_pos = M_pen_poses[M_cursor_pos];
	M_last_set_cursor_pos.first = new_pen_pos.first;

	J_UI_Controller::get_instance().notify_text_box_press(J_View_Shared_t(), get_object_ID()
														  , M_cursor_pos);
}

void J_FT_Text_Display::move_cursor_to_line_begin(){
	auto pen_pos = M_pen_poses[M_cursor_pos];
	pen_pos.first = x1();
	set_cursor_pos(min(get_cursor_index(pen_pos)+1, M_pen_poses.size()-1));
	J_UI_Controller::get_instance().notify_text_box_press(J_View_Shared_t(), get_object_ID()
														  , M_cursor_pos);
}

void J_FT_Text_Display::move_cursor_to_line_end(){
	auto pen_pos = M_pen_poses[M_cursor_pos];
	pen_pos.first = x2();
	set_cursor_pos(get_cursor_index(pen_pos));
	J_UI_Controller::get_instance().notify_text_box_press(J_View_Shared_t(), get_object_ID()
														  , M_cursor_pos);
}

void J_FT_Text_Display::scroll(int i_scroll_val){
	if(!i_scroll_val){
		return;
	}
	
	j_float scroll_size = -i_scroll_val*new_line_screen_size();
	
	if((scroll_size < 0) && M_pen_poses.front() == default_pen_pos()){
		return;
	}

	if(scroll_size < 0){
		j_float scroll_size_to_default 
			= default_pen_pos().second - M_pen_poses.front().second;
		scroll_size = max(scroll_size, scroll_size_to_default);
	} else if(M_pen_poses.back().second > y2()){
		return;
	}else{
		scroll_size = min(scroll_size, y2() - M_pen_poses.back().second);
	}
	auto new_starting_pen_pos = M_pen_poses.front();
	new_starting_pen_pos.second += scroll_size;

	set_starting_pen_pos(new_starting_pen_pos);
	scroll_selection_boxes(0.0f, scroll_size);
}

int J_FT_Text_Display::lines_scrolled_per_tick(){
	return 1;
}

void J_FT_Text_Display::set_cursor_pos_no_scroll(j_size_t i_pos){
	M_cursor_pos = i_pos;
	auto pen_pos_x = M_pen_poses[i_pos].first;
	auto pen_pos_y = M_pen_poses[i_pos].second;



	glBindBuffer(GL_ARRAY_BUFFER, M_cursor_buffer_id);

	std::array<j_float, 4> cursor_data
		= {pen_pos_x, pen_pos_y
		, pen_pos_x, pen_pos_y
		+ new_line_screen_size()
	};

	glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(0), sizeof(cursor_data), cursor_data.data());


	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void J_FT_Text_Display::alert_cursor_pos(Pen_Pos_FL_t i_pos){

	if(!M_left_mouse_button_pressed_status){
		return;
	}
	j_set_cursor_type(J_I_BEAM_CURSOR_ID);
	j_size_t cursor_index = get_cursor_index(i_pos);
	set_cursor_pos(cursor_index);
	j_size_t low_bound = min(cursor_index, M_selection_start_cursor_pos);
	j_size_t right_bound = max(cursor_index, M_selection_start_cursor_pos);



	if(low_bound == right_bound){
		M_selection_boxes.clear();
		return;
	}

	int num_display_boxes = 0;
	

	auto cur_cursor_pos = low_bound;
	while(cur_cursor_pos < right_bound){
		J_Rectangle rectangle;
		rectangle.set_box(M_pen_poses[cur_cursor_pos].first
						  , M_pen_poses[cur_cursor_pos].first
						  , M_pen_poses[cur_cursor_pos].second+ new_line_screen_size() 
						  , M_pen_poses[cur_cursor_pos].second);

		j_float end_x_pos = rectangle.x1();
		while((cur_cursor_pos < right_bound)
			  && (M_pen_poses[cur_cursor_pos].second == rectangle.y2())){
			end_x_pos = M_pen_poses[cur_cursor_pos].first;
			++cur_cursor_pos;
		}
		rectangle.set_x2(end_x_pos);

		if(M_selection_boxes.size() <= num_display_boxes){
			assert(M_selection_boxes.size() == num_display_boxes);

			J_Display_Box_Shared_t display_box(
				new J_Display_Box(0)
				);
			M_selection_boxes.push_back(display_box);
		}
		
		set_selection_box_settings(M_selection_boxes[num_display_boxes]
								   , rectangle);
		
		++num_display_boxes;
	}
	M_selection_boxes.resize(num_display_boxes);
}

void J_FT_Text_Display::set_selection_box_settings(J_Display_Box_Shared_t i_box
												   , J_Rectangle i_rec)const{
	i_rec.set_y(i_rec.y2() - 0.01f);
	i_box->set_box(i_rec);
	i_box->set_fill_visibility_status(true);
	i_box->set_fill_color(J_BLUE*0.4f + J_RED*0.08f);
	i_box->set_outline_color(J_WHITE);

}

void J_FT_Text_Display::scroll_selection_boxes(j_float i_x_scroll, j_float i_y_scroll){
	for(auto f_box : M_selection_boxes){
		f_box->set_x(f_box->x1() + i_x_scroll);
		f_box->set_y(f_box->y2() + i_y_scroll);
	}
}



J_FT_Text_Multi_State_Display::J_FT_Text_Multi_State_Display(j_uint i_obj_id)
	: J_FT_Text_Display_Object(i_obj_id){}

void J_FT_Text_Multi_State_Display::add_char(J_UI_Char i_char){
	M_current_state->add_char(i_char);
}

void J_FT_Text_Multi_State_Display::insert_char(j_size_t i_pos, J_UI_Char i_char){
	M_current_state->insert_char(i_pos, i_char);
}

void J_FT_Text_Multi_State_Display::insert_chars(j_size_t i_pos, const J_UI_String& irk_string){
	M_current_state->insert_chars(i_pos, irk_string);
}

void J_FT_Text_Multi_State_Display::set_string(const J_UI_Multi_String& irk_source){
	M_current_state->set_string(irk_source);
}

void J_FT_Text_Multi_State_Display::delete_char(j_size_t i_pos){
	M_current_state->delete_char(i_pos);
}

void J_FT_Text_Multi_State_Display::erase_chars(j_size_t i_start, j_size_t i_end){
	M_current_state->erase_chars(i_start, i_end);
}

void J_FT_Text_Multi_State_Display::add_string(const J_UI_String& irk_string){
	M_current_state->add_string(irk_string);
}

void J_FT_Text_Multi_State_Display::add_multi_string(const J_UI_Multi_String& irk_string){
	M_current_state->add_multi_string(irk_string);
}

void J_FT_Text_Multi_State_Display::draw()const{
	M_current_state->draw();
}

void J_FT_Text_Multi_State_Display::alert_resize(int i_width, int i_height){
	M_current_state->alert_resize(i_width, i_height);
}

void J_FT_Text_Multi_State_Display::pop_back(){
	M_current_state->pop_back();
}

void J_FT_Text_Multi_State_Display::set_cursor_pos(j_size_t i_size){
	M_current_state->set_cursor_pos(i_size);
}

void J_FT_Text_Multi_State_Display::set_cursor_color(J_Color_RGBA<j_float> i_color){
	M_current_state->set_cursor_color(i_color);
}

void J_FT_Text_Multi_State_Display::mouse_button_press(J_View_Shared_t i_view
	, int i_button, int i_modifiers, Pen_Pos_FL_t i_pen_pos){
	M_current_state->mouse_button_press(i_view, i_button, i_modifiers, i_pen_pos);
}

void J_FT_Text_Multi_State_Display::mouse_button_release(J_View_Shared_t i_view
	, int i_button, int i_modifiers, Pen_Pos_FL_t i_pen_pos){
	M_current_state->mouse_button_release(i_view, i_button, i_modifiers, i_pen_pos);
}

j_size_t J_FT_Text_Multi_State_Display::get_cursor_index(Pen_Pos_FL_t i_pen_pos)const{
	return M_current_state->get_cursor_index(i_pen_pos);
}

void J_FT_Text_Multi_State_Display::add_state(){
	M_text_states.push_back(
		J_FT_Text_Display_Shared_t(new J_FT_Text_Display(get_object_ID(), get_ID()))
		);

}

void J_FT_Text_Multi_State_Display::set_state(j_size_t i_index){
	assert(between_inclusive(i_index, J_SIZE_T_ZERO, M_text_states.size()));
	M_current_state = M_text_states[i_index];
}

void J_FT_Text_Multi_State_Display::set_box(const J_Rectangle& irk_rec){
	M_current_state->set_box(irk_rec);
}

void J_FT_Text_Multi_State_Display::set_fill_color(const J_UI_Color& i_color){
	M_current_state->set_fill_color(i_color);
}

void J_FT_Text_Multi_State_Display::set_outline_color(const J_UI_Color& i_color){
	M_current_state->set_outline_color(i_color);
}

void J_FT_Text_Multi_State_Display::set_fill_visibility_status(bool i_status){
	M_current_state->set_fill_visibility_status(i_status);
}

void J_FT_Text_Multi_State_Display::set_outline_visibility_status(bool i_status){
	M_current_state->set_outline_visibility_status(i_status);
}

void J_FT_Text_Multi_State_Display::set_outline_thickness(j_float i_float){
	M_current_state->set_outline_thickness(i_float);
}

void J_FT_Text_Multi_State_Display::set_cursor_visibility_status(bool i_status){
	M_current_state->set_cursor_visibility_status(i_status);

}

bool J_FT_Text_Multi_State_Display::auto_scrolling_status()const{
	return M_current_state->auto_scrolling_status();

}

void J_FT_Text_Multi_State_Display::set_auto_scrolling_status(bool i_status){
	M_current_state->set_auto_scrolling_status(i_status);
}

bool J_FT_Text_Multi_State_Display::is_cursor_pos_in_view(j_size_t i_pos)const {
	return M_current_state->is_cursor_pos_in_view(i_pos);

}

void J_FT_Text_Multi_State_Display::move_cursor_line_pos_up(j_size_t i_move_val){
	M_current_state->move_cursor_line_pos_up(i_move_val);
}

void J_FT_Text_Multi_State_Display::move_cursor_line_pos_down(j_size_t i_move_val){
	M_current_state->move_cursor_line_pos_down(i_move_val);
}

void J_FT_Text_Multi_State_Display::move_cursor_to_line_begin(){
	M_current_state->move_cursor_to_line_begin();
}

void J_FT_Text_Multi_State_Display::move_cursor_to_line_end(){
	M_current_state->move_cursor_to_line_end();
}

void J_FT_Text_Multi_State_Display::scroll(int i_scroll_val){
	M_current_state->scroll(i_scroll_val);
}

void J_FT_Text_Multi_State_Display::alert_cursor_pos(Pen_Pos_FL_t i_pos){
	M_current_state->alert_cursor_pos(i_pos);
}

}