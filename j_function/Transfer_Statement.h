#ifndef TRANSFER_STATEMENT_H
#define TRANSFER_STATEMENT_H

#include "Expression_Statement.h"


namespace jomike{
class Transfer_Statement : public Expression_Statement{
public:
	using Expression_Statement::Expression_Statement;

	Transfer_Statement* get_copy()const override;

	Transfer_Statement* move_copy()override;


private:

	j_value derived_get_value(const Arguments& i_args)const override;
};

}

#endif //TRANSFER_STATEMENT_H


