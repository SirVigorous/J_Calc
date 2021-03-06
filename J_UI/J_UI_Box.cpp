#include "J_UI_Box.h"
//
#include <J_Shader_Program.h>
//
#include <J_Open_GL.h>

//
#include <iostream>

using std::cerr; using std::cout; using std::endl;
namespace jomike{
static J_Open_GL s_open_gl;
const int SK_VERTEX_ARRAY_SIZE = 24;
J_UI_Box::J_UI_Box(const J_Rectangle& irk_pos):J_Rectangle(irk_pos){
	initialization();
	M_shader = J_GL_Box_Shader_Unique_t(new J_GL_Box_Shader);
	cerr << "\nNew Box Shader";
}

J_UI_Box::J_UI_Box(const J_Rectangle& irk_rectangle, J_GL_Box_Shader* i_box_shader)
	:J_Rectangle(irk_rectangle){
	M_shader = J_GL_Box_Shader_Unique_t(i_box_shader);
	initialization();
}

void J_UI_Box::initialization(){

	init_vaos();
	recalculate_vaos();
}

void J_UI_Box::init_vaos(){
	s_open_gl.bind_vertex_array(M_box_vao);

	s_open_gl.bind_buffer(GL_Buffer_Targets::ARRAY_BUFFER, M_vao_buffer);
	s_open_gl.buffer_data(
		GL_Buffer_Targets::ARRAY_BUFFER, SK_VERTEX_ARRAY_SIZE,
		nullptr, GL_Draw_Types::DYNAMIC);

	s_open_gl.vertix_attribute_pointer(0, 4, GL_Types::FLOAT, false, 0, 0);
	s_open_gl.vertix_attribute_pointer(1, 2, GL_Types::FLOAT, false, 0
									   , sizeof(j_float)* 16);
	s_open_gl.enable_vertex_attribute_array(0);
	s_open_gl.enable_vertex_attribute_array(1);


	s_open_gl.debind_vertex_array();
	s_open_gl.debind_buffer(GL_Buffer_Targets::ARRAY_BUFFER);
}

void J_UI_Box::recalculate_vaos(){
	std::array<j_float, SK_VERTEX_ARRAY_SIZE> vertex_array_data = {
		x1(), y1(), 0.0f, 1.0f
		, x2(), y1(), 0.0f, 1.0f
		, x2(), y2(), 0.0f, 1.0f
		, x1(), y2(), 0.0f, 1.0f

		, 0.0f, 0.0f
		, 1.0f, 0.0f
		, 1.0f, 1.0f
		, 0.0f, 1.0f
	};




	s_open_gl.bind_vertex_array(M_box_vao);

	s_open_gl.bind_buffer(GL_Buffer_Targets::ARRAY_BUFFER, M_vao_buffer);
	assert(vertex_array_data.size() == SK_VERTEX_ARRAY_SIZE);
	s_open_gl.buffer_sub_data(
		GL_Buffer_Targets::ARRAY_BUFFER, 0, SK_VERTEX_ARRAY_SIZE,
		vertex_array_data.data());




	s_open_gl.debind_vertex_array();
	s_open_gl.debind_buffer(GL_Buffer_Targets::ARRAY_BUFFER);

}

void J_UI_Box::draw()const{
	s_open_gl.bind_vertex_array(M_box_vao);

	if(fill_visibility_status()){
		s_open_gl.use_program(M_shader->box_program_id());
		s_open_gl.draw_arrays(Array_Draw_Mode::TRIANGLE_FAN, 0, 4);
	}

	if(outline_visibility_status()){

		s_open_gl.use_program(M_shader->outline_program_id());
		s_open_gl.line_width(outline_thickness());
		s_open_gl.draw_arrays(Array_Draw_Mode::LINE_LOOP, 0, 4);

	}
	s_open_gl.debind_vertex_array();
	s_open_gl.debind_program();


}


//J_UI_Box::J_UI_Box(const J_Rectangle& irk_pos, j_uint i_obj_id): J_UI_Object(i_obj_id)
//, J_Rectangle(irk_pos){}

void J_UI_Box::set_box(j_float i_x1, j_float i_x2, j_float i_y1, j_float i_y2){
	J_Rectangle::set_box(i_x1, i_y2, i_x2 - i_x1, i_y1 - i_y2);
	recalculate_vaos();
}

void J_UI_Box::derived_set_fill_color(){
	M_shader->set_fill_color(fill_color());
}

void J_UI_Box::derived_set_outline_color(){
	M_shader->set_outline_color(outline_color());
}

void J_UI_Box::set_width(j_float i_width){
	J_Rectangle::set_width(i_width);
	recalculate_vaos();
}

void J_UI_Box::set_height(j_float i_height){
	J_Rectangle::set_height(i_height);
	recalculate_vaos();
}

void J_UI_Box::set_x(j_float i_x){
	J_Rectangle::set_x(i_x);
	recalculate_vaos();
}

void J_UI_Box::set_y(j_float i_y){
	J_Rectangle::set_y(i_y);
	recalculate_vaos();
}

void J_UI_Box::set_rectangle(const J_Rectangle& irk_rectangle){
	J_Rectangle::set_rectangle(irk_rectangle);
	recalculate_vaos();

}

J_UI_Box::~J_UI_Box(){

}

void J_UI_Box::alert_resize(int, int){

}

const J_GL_Vertex_Array& J_UI_Box::get_box_vao()const{
	return M_box_vao;
}


}



