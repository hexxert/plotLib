#ifndef DRAWH
	#define DRAWH
		/*
		 * include modules src files here
		 */
		#include <GL/glut.h>
		#include "../modules/modules.h"
	 	#include "interface.h"
		
		/* the draw and idle function called by glut 
 		 * are defined in this file. They are responsible
		 * for all drawing commands: meaning calling the required
		 * modules from the modules folder...
		 */
	 	void plt_idle(void);
	 	void plt_draw(void);
	 	void plt_reshape_window(int width, int height);
#endif
