#ifndef J_UI_FUNCTION_H
#define J_UI_FUNCTION_H
#include "J_UI_Fwd_Decl.h"
#include "../J_Utile/J_Fwd_Decl.h"

namespace jomike{

class J_Display_Box_Position_FO{
public:
	bool operator()(const J_Display_Box_Shared_t&, const J_Display_Box_Shared_t&)const;
	bool operator()(Pen_Pos_FL_t i_pen_pos, const J_Display_Box_Shared_t&)const;
	bool operator()(const J_Display_Box_Shared_t&, Pen_Pos_FL_t i_pen_pos)const;
};


}


#endif