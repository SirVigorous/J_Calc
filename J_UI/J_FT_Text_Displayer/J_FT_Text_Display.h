#ifndef J_FT_TEXT_DISPLAY_H
#define J_FT_TEXT_DISPLAY_H
#include <J_Error.h>
#include <J_Image.h>
#include "../J_Display_Object/J_Display_Object.h"
#include "../J_UI_String/J_UI_String.h"
//Containers
#include <array>
#include <ex_array.h>
#include <utility>

#include <mutex>
namespace jomike{


class J_FT_Text_Display_Object : public J_Display_Box{
public:
	J_FT_Text_Display_Object(j_uint);
	virtual void add_char(J_UI_Char i_char) = 0;
	virtual void insert_char(j_size_t i_pos, J_UI_Char i_char) = 0;
	virtual void insert_chars(j_size_t i_pos, const J_UI_String&) = 0;
	virtual void set_string(const J_UI_Multi_String& irk_source) = 0;
	virtual void delete_char(j_size_t i_pos) = 0;
	virtual void erase_chars(j_size_t i_start, j_size_t i_end) = 0;
	virtual void add_string(const J_UI_String&) = 0;
	virtual void add_multi_string(const J_UI_Multi_String&) = 0;
	virtual void set_cursor_visibility_status(bool i_status) = 0;
	virtual void pop_back() = 0;
	virtual void set_cursor_pos(j_size_t i_size) = 0;

	virtual void move_cursor_line_pos_up(j_size_t i_move_val) = 0;
	virtual void move_cursor_line_pos_down(j_size_t i_move_val) = 0;
	virtual void move_cursor_to_line_begin() = 0;
	virtual void move_cursor_to_line_end() = 0;

	virtual void set_cursor_color(J_Color_RGBA<j_float>) = 0;
	virtual j_size_t get_cursor_index(Pen_Pos_FL_t)const = 0;
	virtual void scroll(int i_scroll_val)=0;
	virtual bool auto_scrolling_status()const = 0;
	virtual void set_auto_scrolling_status(bool) = 0;
	virtual bool is_cursor_pos_in_view(j_size_t) const = 0;
	~J_FT_Text_Display_Object();

protected:
	J_FT_Text_Display_Object(j_uint, j_uint);
private:
	
};

class J_FT_Text_Multi_State_Display : public J_FT_Text_Display_Object{
public:
	J_FT_Text_Multi_State_Display(j_uint);

	//void add_font_face(J_Font_Face);
	void add_char(J_UI_Char i_char)override;
	void insert_char(j_size_t i_pos, J_UI_Char i_char)override;
	void insert_chars(j_size_t i_pos, const J_UI_String&)override;
	void set_string(const J_UI_Multi_String& irk_source)override;
	void delete_char(j_size_t i_pos)override;
	void erase_chars(j_size_t i_start, j_size_t i_end)override;
	void add_string(const J_UI_String&)override;
	void add_multi_string(const J_UI_Multi_String&)override;
	void draw()const override;
	void alert_resize(int, int)override;
	void pop_back()override;
	void set_cursor_pos(j_size_t i_size)override;
	void set_cursor_color(J_Color_RGBA<j_float>)override;
	void mouse_button_press(J_View_Shared_t, int i_button, int modifiers, Pen_Pos_FL_t)override;
	void mouse_button_release(J_View_Shared_t, int i_button, int modifiers, Pen_Pos_FL_t)override;

	void set_box(const J_Rectangle& irk_rec)override;

	void alert_cursor_pos(Pen_Pos_FL_t i_pos);

	j_size_t get_cursor_index(Pen_Pos_FL_t)const override;

	void add_state();
	void set_state(j_size_t i_index);

	void set_fill_color(const J_UI_Color& i_color);
	void set_outline_color(const J_UI_Color& i_color);

	void set_fill_visibility_status(bool i_status);
	void set_outline_visibility_status(bool i_status);

	void set_outline_thickness(j_float)override;



	void set_cursor_visibility_status(bool i_status)override;

	bool auto_scrolling_status()const override;

	void set_auto_scrolling_status(bool)override;

	bool is_cursor_pos_in_view(j_size_t)const override;

	void move_cursor_line_pos_up(j_size_t i_move_val)override;

	void move_cursor_line_pos_down(j_size_t i_move_val)override;

	void move_cursor_to_line_begin()override;

	virtual void move_cursor_to_line_end();
	void scroll(int i_scroll_val)override;
private:
	ex_array<J_FT_Text_Display_Shared_t> M_text_states;
	J_FT_Text_Display_Shared_t M_current_state;


};

class J_FT_Text_Display : public J_FT_Text_Display_Object{
public:
	J_FT_Text_Display(j_uint);
	J_FT_Text_Display(j_uint i_object_id, j_uint i_id);

	//void add_font_face(J_Font_Face);
	void add_char(J_UI_Char i_char)override;
	void insert_char(j_size_t i_pos, J_UI_Char i_char)override;
	void insert_chars(j_size_t i_pos, const J_UI_String&)override;
	void set_string(const J_UI_Multi_String& irk_source)override;
	void delete_char(j_size_t i_pos)override;
	void erase_chars(j_size_t i_start, j_size_t i_end)override;
	void add_string(const J_UI_String&)override;
	void add_multi_string(const J_UI_Multi_String&)override;
	void draw()const override;
	void alert_resize(int,int)override;
	void pop_back()override;
	void set_cursor_pos(j_size_t i_size)override;

	void set_cursor_pos_no_scroll(j_size_t i_pos);

	virtual void move_cursor_line_pos_up(j_size_t i_move_val);

	virtual void move_cursor_line_pos_down(j_size_t i_move_val);
	void set_cursor_color(J_Color_RGBA<j_float>)override;
	void mouse_button_press(J_View_Shared_t, int i_button, int modifiers, Pen_Pos_FL_t)override;
	void mouse_button_release(J_View_Shared_t, int i_button, int modifiers, Pen_Pos_FL_t)override;
	void set_cursor_visibility_status(bool i_status)override;
	j_size_t get_cursor_index(Pen_Pos_FL_t)const override;


	bool cursor_visibility_status()const;
	bool M_cursor_visibility_status;

	bool auto_scrolling_status()const;
	void set_auto_scrolling_status(bool i_status);
	
	void alert_cursor_pos(Pen_Pos_FL_t);

	bool is_cursor_pos_in_view(j_size_t i_pos)const override;
	void move_cursor_to_line_begin()override;
	void scroll(int i_scroll_val)override;
	void move_cursor_to_line_end()override;
	~J_FT_Text_Display();
protected:
	
private:
	//Render Settings
	J_Text_Shader_Program* M_shader;
	j_uint M_program_id;
	void J_FT_Text_Display::draw_pos(j_size_t i_pos)const;

	void draw_string(J_UI_Multi_String::const_iterator)const;
	
	J_UI_Multi_String M_string;
	ex_array<j_uint> M_vao_ids;
	ex_array<j_uint> M_vao_buffer_ids;
	ex_array<Pen_Pos_FL_t> M_pen_poses;
	int M_new_line_size;
	j_size_t M_cursor_pos;
	j_uint M_cursor_vao_id;
	j_uint M_cursor_buffer_id;
	
	void init_cursor_vao();
	
	void draw_to_buffer(j_size_t index)const;
	void draw_cursor()const;
	Pen_Pos_FL_t default_pen_pos()const;
	int lines_scrolled_per_tick();
	Pen_Pos_FL_t calculate_pen_advance(Pen_Pos_FL_t i_cur_pen, int i_advance)const;
	void recalculate_text_positions();
	void clear();
	void clear_from(j_size_t pos);
	void set_starting_pen_pos(Pen_Pos_FL_t);
	void delete_vaos();
	void delete_buffers();
	void delete_vaos_from(j_size_t pos);
	void delete_buffers_from(j_size_t pos);
	void add_new_vao_and_vbo(int);
	Pen_Pos_FL_t new_line_pen_pos(Pen_Pos_FL_t i_cur_pen)const;
	j_float new_line_screen_size()const;
	void auto_scroll_window(j_size_t i_pos);
	void set_selection_box_settings(J_Display_Box_Shared_t, J_Rectangle)const;
	void scroll_selection_boxes(j_float i_x_scroll, j_float i_y_scroll);
	bool M_left_mouse_button_pressed_status = false;


	Pen_Pos_FL_t M_last_set_cursor_pos = default_pen_pos();
	
	ex_array<J_Display_Box_Shared_t> M_selection_boxes;

	j_uint Test_cursor_buffer;
	bool M_auto_scrolling_status = true;
	j_size_t M_selection_start_cursor_pos;


};





}

#endif