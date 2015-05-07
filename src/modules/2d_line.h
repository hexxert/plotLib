#ifndef LINE2D_H
	#define LINE2D_H
	
	/*
	 * specify the object controlling the layer properties
	 */
	enum PLT_LINETYPE{ plt_LineType_NONE };
	enum PLT_SYMBOLTYPE{ plt_SymbolType_NONE };
	typedef struct prop_2d_layer{
			uint32_t 	color;
			int32_t 	line_type;
			int32_t 	line_width;
			int32_t 	line_symbol;
			int32_t 	color_map;
			}prop_2d_layer_t;	
	
	static void * plt_2d_line_aux(int32_t layer_num);
	static int32_t plt_2d_line_req(plot_t * plt, const double * x, const double * y, const double * z);
	static int32_t plt_2d_line_draw(void *, const double * x, const double * y, const double * z, int32_t layer, int32_t num_layers);
	static void plt_2d_line_layer(int32_t property, int32_t value);
	static void plt_2d_line_free_aux(void * ptr);	
	
#endif
