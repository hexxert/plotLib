#include "3d_line.h"
/* 
 *
 *
 */
void * plt_3d_line_aux(int32_t layer_num){
	


}


int32_t plt_3d_line_req(plot_t * plt, const double * x, const double * y, const double * z){
	/* two lines of data are required  */
	C_CHECK_CONDITION( x == NULL, API_DATA_NOT_VALID);
	C_CHECK_CONDITION( y == NULL, API_DATA_NOT_VALID);
	C_CHECK_CONDITION( z == NULL, API_DATA_NOT_VALID);
	return 0;
error:
	return -1;

}


int32_t plt_3d_line_draw(void * ptr, const double * x, const double * y, const double * z, int32_t layer, int32_t num_layers){


}

void plt_3d_line_layer(int32_t property, int32_t value){


}


void plt_3d_line_free_aux(void * ptr){



}
