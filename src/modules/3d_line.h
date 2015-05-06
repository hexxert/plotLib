#ifndef LINE3D_H
	#define LINE3D_H
	static void * plt_3d_line_aux(int32_t layer_num);
	static int32_t plt_3d_line_req(plot_t * plt, const double * x, const double * y, const double * z);
	static int32_t plt_3d_line_draw(void *, const double * x, const double * y, const double * z, int32_t layer, int32_t num_layers);
	static void plt_3d_line_layer(int32_t property, int32_t value);
	static void plt_3d_line_free_aux(void * ptr);
#endif