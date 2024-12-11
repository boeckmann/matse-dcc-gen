#include "train.h"
#include "stream.h"

int system_reset( void )
{
	trains_remove_all();
	stream_dcc_reset();
	return 1;
}
