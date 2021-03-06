#include "J_UI_Controller.h"
#include "J_Font_Manager.h"
//Algorithm
#include <algorithm>
#include <functional>
using std::bind; using std::for_each; using std::dynamic_pointer_cast;
using namespace std::placeholders;
//
#include "J_View.h"
//
#include "J_UI_Object.h"
//
#include "J_FT_Text_Displayer/J_Text_Box.h"
//
#include <J_OpenGl_Functions.h>
//
#include <iostream>
//
#include <iomanip>
//
#include <fstream>
//
#include <J_Frame_Counter.h>
//
#include <sstream>
//
#include "J_UI.h"

using std::ifstream; using std::string; using std::cerr; using std::getline;
using std::ostringstream; using std::setw; using std::istream; using std::endl;
namespace jomike{

const j_dbl DRAW_REFRESH_TIME = 1.0/5.0;
static j_dbl draw_refresh_time(){ return DRAW_REFRESH_TIME; }

static Instance_Pointer<J_Font_Manager> s_font_manager;
static Instance_Pointer<J_UI_Controller> s_controller;
static Instance_Pointer<J_UI> s_j_ui;
void J_UI_Controller::init(int argc, char** argv){
	derived_init(argc, argv);
}

void J_UI_Controller::initialize_text_data(){
	s_font_manager->add_font("Fonts/times.ttf", "times");
	s_font_manager->add_font("Fonts/timesi.ttf", "times_italic");
	s_font_manager->add_font("Fonts/timesbd.ttf", "times_bold");
	s_font_manager->add_font("Fonts/timesbi.ttf", "times_bold_italic");
	s_font_manager->add_font("Fonts/machadomath.ttf", "math_symbols");
	s_font_manager->add_font("Fonts/Consolas.ttf", "consolas");
}

J_UI_Controller::~J_UI_Controller(){}

void J_UI_Controller::clear_all(){
	auto view_start = make_pair_iter(M_j_views.begin());
	auto view_end = make_pair_iter(M_j_views.end());


	s_font_manager->clear();
	//for_each(view_start, view_end, mem_fn(&J_View::clear));
	M_j_views.clear();
}


void J_UI_Controller::add_view(J_View_Shared_t i_view){
	assert(!M_j_views.count(i_view->get_window()));

	M_j_views[i_view->get_window()] = i_view;
}

void J_UI_Controller::remove_view(j_window_t i_window){
	assert(M_j_views.count(i_window));

	M_j_views.erase(i_window);
}

bool J_UI_Controller::has_views()const{
	return !M_j_views.empty();
}

void J_UI_Controller::draw_views()const{
	for (auto view : M_j_views){
		view.second->draw();
	}

	M_frame_counter->count_frame();
	

}

void J_UI_Controller::char_input_cmd(j_window_t i_window, int i_charcode){

	assert(M_j_views.count(i_window));
	M_j_views[i_window]->char_input_cmd(i_charcode);
}

void J_UI_Controller::key_input_cmd(j_window_t i_window, int i_key, int i_scancode, int i_action, int i_modifiers){
	M_last_key_modifiers = i_modifiers;

	assert(M_j_views.count(i_window));

	M_j_views[i_window]->key_input_cmd(i_key, i_scancode, i_action, i_modifiers);
}


void J_UI_Controller::mouse_button_cmd(j_window_t i_window, int i_mouse_key
	, int i_action, int i_modifiers){
	M_last_key_modifiers = i_modifiers;
	auto found_pos = M_j_views.find(i_window);
	assert(M_j_views.end() != found_pos);

	J_View_Shared_t view = found_pos->second;
	assert(view->get_window() == i_window);
	if(J_PRESS == i_action){
		view->mouse_button_press(view, i_mouse_key
								 , i_modifiers
								 , s_j_ui->cursor_pos(view).cursor_pos_fl());
	} else if(J_RELEASE == i_action){
		view->mouse_button_release(view, i_mouse_key
								 , i_modifiers
								 , s_j_ui->cursor_pos(view).cursor_pos_fl());
	}






}



void J_UI_Controller::mouse_button_cmd_n(j_window_t i_window, int i_mouse_key
									   , int i_action, int i_modifiers, int i_count){
	M_last_key_modifiers = i_modifiers;
	auto found_pos = M_j_views.find(i_window);
	assert(M_j_views.end() != found_pos);

	J_View_Shared_t view = found_pos->second;
	assert(view->get_window() == i_window);
	if(J_PRESS == i_action){
		view->mouse_button_press_n(view, i_mouse_key
								 , i_modifiers
								 , s_j_ui->cursor_pos(view).cursor_pos_fl(), i_count);
	} else if(J_RELEASE == i_action){
		assert(!"Multiple releases?");

	}






}


J_UI_Controller::J_UI_Controller(){
	initialize_text_data();
	M_frame_counter = J_Frame_Counter_Shared_t(new J_Frame_Counter(5));
}

j_dbl cursor_update_time(){
	return 1.0 / 60.0;
}

void J_UI_Controller::cursor_pos_input_cmd(j_window_t i_window, j_dbl i_x_pos, j_dbl i_y_pos){


	assert(M_j_views.count(i_window));
	auto view = M_j_views[i_window];
	s_j_ui->set_cursor_pos(view, i_x_pos, i_y_pos);

	static 	J_Duration_Tester<j_dbl(*)(void), j_dbl(*)()>
		draw_timer(get_j_ui_time, cursor_update_time);

	if(!draw_timer.time_exceeded()){
		return;
	}

	draw_timer.reset_timer();


	view->set_cursor_pos(safe_int_cast(i_x_pos), safe_int_cast(i_y_pos));
}

J_FWD_DECL(Cursor_Pos_Updater)

class Cursor_Pos_Updater : public J_UI_Object_Update_Callback{
public:
	Cursor_Pos_Updater(J_View_Weak_t i_view):M_view(i_view){}
	void operator()(J_UI_Object_Shared_t i_obj)override;
private:
	Pen_Pos_FL_t M_cursor_pos_fl;
	J_View_Weak_t M_view;
};

void Cursor_Pos_Updater::operator()(J_UI_Object_Shared_t i_obj){

	if(s_j_ui->cursor_pos(M_view).cursor_pos_fl() == M_cursor_pos_fl){
		return;
	}
	
	M_cursor_pos_fl = s_j_ui->cursor_pos(M_view).cursor_pos_fl();
	auto text_box = dynamic_pointer_cast<J_Text_Box_Object>(i_obj);
	assert(text_box);
	ostringstream o_str;
	auto width_manip = setw(8);
	o_str.precision(3);
	o_str << std::fixed;
	o_str << '\t' << M_cursor_pos_fl.first
		<< " : " << M_cursor_pos_fl.second;

	auto font_face = text_box->get_string().front().font_face();
	auto color = text_box->get_string().front().color();
	text_box->set_string(J_UI_String(o_str.str(), font_face, color));
	//std::cout << "\n" << o_str.str() << std::endl;
}

void J_UI_Controller::add_cursor_pos_text_updater(J_Text_Box_Object_Shared_t i_text_obj
	, J_View_Weak_t i_view){
	i_text_obj->add_update_callback(Cursor_Pos_Updater_Shared_t(new Cursor_Pos_Updater(i_view)));
}

void J_UI_Controller::defocus_active_object(j_window_t i_window){
	auto found_pos = M_j_views.find(i_window);
	if(M_j_views.end() == found_pos){
		return;
	}
	auto found_obj = M_active_ui_objs.find(found_pos->second);

	if(M_active_ui_objs.end() == found_obj){
		return;
	}
	if(found_obj->second.expired()){
		return;
	}
	found_obj->second.lock()->set_focus_status(false);
}



void J_UI_Controller::resize_cmd(j_window_t i_window, int i_width, int i_height){
	auto found_view_pos = M_j_views.find(i_window);
	assert(M_j_views.end() != found_view_pos);

	found_view_pos->second->resize(i_width, i_height);
}



static void mouse_press_script_cmd(J_UI_Controller*, istream&);
static void char_press_script_cmd(J_UI_Controller*, istream&);
static void mouse_release_script_cmd(J_UI_Controller*, istream&);
static void end_script_command(J_UI_Controller* i_controller, istream&){
	i_controller->end_script_run();
}

static void process_script_cmd(J_UI_Controller* i_controller, istream&);
void J_UI_Controller::run_script(const std::string& irk_file_name){
	cerr << "\nSize of Ptr: " << sizeof(void*);
	typedef  std::map<std::string, void(*)(J_UI_Controller*, istream&)>
		Script_Command_Map_t;
	M_script_run_flag = true;
	static Script_Command_Map_t script_commands = {
		{"mouse_press", mouse_press_script_cmd}
		,{"mouse_release", mouse_release_script_cmd}
		,{"char_press", char_press_script_cmd}
		, {"process", process_script_cmd}
		,{"end", end_script_command}
	};

	ifstream file(irk_file_name);

	if(!file){
		cerr << "Invalid Test Script" << endl;
	}

	J_Duration_Tester<j_dbl(*)(void), j_dbl(*)()>
		draw_timer(j_get_time, draw_refresh_time);
	try{
		while(file){
			string command;
			file >> command;
			auto found_command = script_commands.find(command);
			if(script_commands.end() == found_command){
				cerr << "\nBad Script Command Found";
				return;
			}
			(found_command->second)(this, file);

			j_poll_events();

			if(!has_views()){
				break;
			}

			s_j_ui->update();
			for(auto f_view : M_j_views){
				f_view.second->update();
			}


			if(draw_timer.time_exceeded()){
				draw_views();
				draw_timer.reset_timer();
			}

			if(!M_script_run_flag){
				break;
			}
		}

		char_input_cmd(Contexts_Handler::get_instance().get_active_window(), '\n');
	} catch(J_Error& e){
			e.print();
			M_script_run_flag = false;
	}
	M_script_run_flag = false;
	cerr << "\nEnd Script Running" << endl;
}

j_dbl J_UI_Controller::fps()const{
	assert(M_frame_counter);
	return M_frame_counter->fps();
}

J_FWD_DECL(Draw_Fps_Updater)

class Draw_Fps_Updater : public J_UI_Object_Update_Callback{
public:
	void operator()(J_UI_Object_Shared_t i_obj)override;
private:
};

void Draw_Fps_Updater::operator()(J_UI_Object_Shared_t i_obj){
	auto text_box = dynamic_pointer_cast<J_Text_Box_Object>(i_obj);
	assert(text_box);
	ostringstream o_str;
	auto width_manip = setw(8);
	o_str.precision(4);
	o_str << std::fixed;
	o_str << "Draw FPS: " << s_controller->fps();

	auto font_face = text_box->get_string().front().font_face();
	auto color = text_box->get_string().front().color();
	text_box->set_string(J_UI_String(o_str.str(), font_face, color));
}

void J_UI_Controller::add_draw_fps_updater(J_Text_Box_Object_Shared_t i_text_obj){
	i_text_obj->add_update_callback(Draw_Fps_Updater_Shared_t(new Draw_Fps_Updater));
}


class Model_Update_Fps_Updater : public J_UI_Object_Update_Callback{
public:
	void operator()(J_UI_Object_Shared_t i_obj)override{
		auto text_box = dynamic_pointer_cast<J_Text_Box_Object>(i_obj);
		assert(text_box);
		ostringstream o_str;
		auto width_manip = setw(8);
		o_str.precision(4);
		o_str << std::fixed;
		o_str << "FPS: " << s_j_ui->fps();

		auto font_face = text_box->get_string().front().font_face();
		auto color = text_box->get_string().front().color();
		text_box->set_string(J_UI_String(o_str.str(), font_face, color));
	}
};

J_FWD_DECL(Model_Update_Fps_Updater)

void J_UI_Controller::add_update_fps_updater(J_Text_Box_Object_Shared_t i_text_obj){
	i_text_obj->add_update_callback(
			Model_Update_Fps_Updater_Shared_t(new Model_Update_Fps_Updater)
		);
	
}

void J_UI_Controller::end_script_run(){
	M_script_run_flag = false;
}

int J_UI_Controller::current_key_modifiers()const{
	return M_last_key_modifiers;
}

void J_UI_Controller::update(){
	for(auto f_view : M_j_views){
		f_view.second->update();
	}
}



static j_dbl get_screen_pos_double(istream& ir_is){
	j_dbl i_pos;
	if(!(ir_is >> i_pos)){
		throw J_Syntax_Error("Could not read a double");
	}
	if(!between_inclusive(i_pos, -1.0, 1.0)){
		throw J_Syntax_Error("Mouse Pos not within screen range");
	}
	return i_pos;
}

static void mouse_press_script_cmd(J_UI_Controller* i_controller, istream& ir_stream){
	auto window = Contexts_Handler::get_instance().get_active_window();
	j_focus_context(Contexts_Handler::get_instance().get_active_context());
	j_dbl i_x_pos = get_screen_pos_double(ir_stream);
	j_dbl i_y_pos = get_screen_pos_double(ir_stream);
	j_set_cursor_pos(window, i_x_pos, i_y_pos);
	i_controller->cursor_pos_input_cmd(window, get_x_pixel(window, i_x_pos)
		, get_y_pixel(window, i_y_pos));
	i_controller->mouse_button_cmd(window, J_LEFT_MOUSE_BUTTON, J_PRESS, 0);
	i_controller->draw_views();
}

static void mouse_release_script_cmd(J_UI_Controller* i_controller, istream& ir_stream){
	auto window = Contexts_Handler::get_instance().get_active_window();
	j_focus_context(Contexts_Handler::get_instance().get_active_context());
	j_dbl i_x_pos = get_screen_pos_double(ir_stream);
	j_dbl i_y_pos = get_screen_pos_double(ir_stream);
	j_set_cursor_pos(window, i_x_pos, i_y_pos);
	i_controller->cursor_pos_input_cmd(window, get_x_pixel(window, i_x_pos)
									   , get_y_pixel(window, i_y_pos));
	i_controller->mouse_button_cmd(window, J_LEFT_MOUSE_BUTTON, J_RELEASE, 0);
	i_controller->draw_views();
}

static void process_script_cmd(J_UI_Controller* i_controller, istream& ){
	auto window = Contexts_Handler::get_instance().get_active_window();
	i_controller->key_input_cmd(window, J_KEY_ENTER, 0, J_PRESS, J_MOD_SHIFT);
}

static void char_press_script_cmd(J_UI_Controller* i_controller, istream& ir_stream){
	static string total_string;
	static J_Modder<1000> modder;
	static int num_lines = 0;
	static J_Duration_Tester<j_dbl(*)(void), j_dbl(*)()>
		line_writer_timer(j_get_time, draw_refresh_time);

	string char_string;
	getline(ir_stream, char_string, '@');
	assert(char_string.front() == ' ');
	
	char_string.front() = '\n';

	if(!ir_stream){
		throw J_Syntax_Error("Stream Went Bad in Char Press Read. Possibly no Endline Char");
	}
	if(char_string.empty()){
		throw J_Syntax_Error("Empty Char Press String Encountered");
	}
	auto window = Contexts_Handler::get_instance().get_active_window();
	
	for(int i = 0; i < char_string.size(); i++){
		if(modder.tick()){
			cerr << "\nInserted Char: " << i << ": " << char_string[i] << " of total size: "
				<< char_string.size() << " Percent: " << 100.0*i/char_string.size() << '%';
		}
		i_controller->char_input_cmd(window, char_string[i]);
		if(line_writer_timer.time_exceeded()){
			i_controller->draw_views();
			s_j_ui->update();
			i_controller->update();
			line_writer_timer.reset_timer();
			j_poll_events();
		}
	}
	total_string += char_string;
	++num_lines;
	if(modder.tick()){
		cerr << "\nLine number: " << num_lines << " Total Size: " << total_string.size()
			<< " Time of Run: " << line_writer_timer.elapsed_time() 
			<< " Model Fps: " << s_j_ui->fps();
		line_writer_timer.reset_timer();
	}
	process_script_cmd(i_controller, ir_stream);
}

}