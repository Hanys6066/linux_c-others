#ifndef GRAPH_H
#define GRAPH_H

#include <stdint.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../shaders/shaders.h"

/*________________________Figure Parameters___________________________________*/
#define G_FIG_COLOR          (struct Color){0.80f, 0.80f, 0.80f}

/*________________________Plots Parameters_____________________________________*/
#define G_PLOT_COLOR         (struct Color){0.90f, 0.90f, 0.90f}

/*_____________________________Axis Parameters________________________________*/
#define G_X_AXIS_X1          0.1f
#define G_X_AXIS_Y1          0.1f
#define G_X_AXIS_X2          0.95f
#define G_X_AXIS_Y2          0.1f

#define G_Y_AXIS_X1          0.1f
#define G_Y_AXIS_Y1          0.1f
#define G_Y_AXIS_X2          0.1f
#define G_Y_AXIS_Y2          0.95f

#define G_AXIS_R0_X          0.0f
#define G_AXIS_R1_X          0.2f
#define G_AXIS_R0_Y          0.2f
#define G_AXIS_R1_Y          0.2f

#define G_AXIS_RANGE         {0.0f, 10.0f}

#define G_AXIS_POINTS_1      10 /*axis longer than 500*/
#define G_AXIS_DIG_1         3 /*numbers of digit*/
#define G_AXIS_LINES_SIZE_1  10.0f /*lines size for axis longer than 500 */
#define G_AXIS_TEXT_SCALE_1  1.3f
#define G_AXIS_TEXT_WIDTH_1  1.4f

#define G_AXIS_POINTS_2      5  /*axis longer than 250*/
#define G_AXIS_DIG_2         3  /*numbers of digits*/
#define G_AXIS_LINES_SIZE_2  8.0f /*lines size for axis longer than 250 */
#define G_AXIS_TEXT_SCALE_2  1.0f
#define G_AXIS_TEXT_WIDTH_2  1.2f

#define G_AXIS_POINTS_3      2  /*axis longer than 100 and smaller than 250*/
#define G_AXIS_DIG_3         3  /*numbers of digit*/
#define G_AXIS_LINES_SIZE_3  6.0f /*lines size for axis longer than 100 */
#define G_AXIS_TEXT_SCALE_3  0.8f
#define G_AXIS_TEXT_WIDTH_3  1.2f

#define G_AXIS_WIDTH         1.7f  /*also valid for lines on axis*/
#define G_AXIS_DIR           G_AX_LEFT
#define G_AXIS_COLOR         (struct Color){0.0f, 0.0f, 0.0f}


/*______________________________Line Parameters________________________________*/
#define G_LINE_WIDTH         2.0f       /*default line width*/
#define G_LINE_POINTS_SIZE   4.0f       /*default size of points*/

#define G_FULL_LINE          0xFFFF  /*full line  ------------*/
#define G_DOTTED             0x1010  /*dotted lin ............*/
#define G_DASH               0x00FF  /*dash line  - - - - - -*/
#define G_DASH_DOT           0x1C47  /*dash-dot-dash .-.-.-.-.*/



/*_____________________________Others Parameters______________________________*/
#define G_WHEEL_SIZE          10      /*size of color wheel*/

typedef enum{
    G_AX_LEFT,
    G_AX_RIGHT
}DIR_S_TO_E;        /*direction for axis lines from line(start->end)*/

#ifndef PIX_OBJECT_LIB_H
    struct Color{
            float               r;
            float               g;
            float               b;
    };
#endif

#ifndef ERROR_H
    typedef enum{
        FALSE,
        TRUE
    }Boolean;
#endif

/*axis type*/
typedef enum{
        G_X_AXIS,
        G_Y_AXIS,
        G_O_AXIS        /*for other axis */
}G_AXIS_TYPE;

struct g_rec_cord{            /*rectange coordinance, also used for axis*/
        float               x;
        float               y;
        float               w;                      /*in case of axis defined x2(not width)*/
        float               h;                      /*in case of axis defined y2(not height)*/
};

struct g_xy_cord{     /*Points coordinace*/
        float               x;
        float               y;
};

struct g_title{
        char                *text;
        float               x;
        float               y;
};

struct g_line{
        struct g_title      label;
        unsigned int        num_points; /*number of points*/
        float               *x_data;    /*y data*/
        float               *y_data;    /*x-data*/
        float               width;      /*line width*/
        float               pSize;      /*size of points*/
        GLushort            l_patern;   /*line patern*/
        struct Color        color;
        struct g_xy_cord    *oCord;      /*relative coordinace within axis, only range[0,1] will be showed...*/
        struct g_xy_cord    *tCord;     /*true cord(in range of actual size of window)->real coordinace, can be
                                        different compare to oCord->due to rescaling ...*/
};

struct g_axis{
        G_AXIS_TYPE         type;       /*axis type*/
        struct g_rec_cord   oCord;      /*relative coordinace->range<0,1>*/
        struct g_rec_cord   tCord;      /*absolute coordinace in plot space*/
        Boolean             fix_lines;  /*if true(default FALSE)->number of lines and realtive osition doesnt changed->also is not fit on data*/
        float               tg;         /*tangent of line->in ces when axis is not orthogonal to plot*/
        float               s_x;        /*step in x direction*/
        float               s_y;        /*step in y direction*/
        float               lines_size; /*size of lines of axis*/
        float               tx_width;   /*text wigth(how bold is text)*/
        float               tx_scale;   /*scale of default size of GLUT_STROKE_ROMAN*/
        DIR_S_TO_E          dir;        /*direction in which lines on axis will be draw*/
        float               width;      /*line width*/
        unsigned int        num_n;      /*size of numbers array*/
        unsigned int        digits;     /*number of digit*/
        char                **numbers;  /*axis points(values)*/
        char                *label;     /*axis label*/
         /*if fix_lines == TRUE, this value is use for range computation: x range[0] = min * (min > 0 ? (1.0f - r0): (1.0f + r0))*/
        float               r0;
        /*if fix_lines == TRUE, this value is use for range computation: x range[1] = max * (max > 0 ? ((1.0f + r1): (1.0f - r1))*/
        float               r1;
        float               range[2];   /*min and max value of axis*/
        struct Color        color;      /*axis range*/
};

struct g_plot{
        struct g_title      title;      /*title of plot->will be placed at the top of plot*/
        int                 id;         /*index of actaul plot in g_figure.plots array*/
        uint8_t             num_lines;  /*number of lines*/
        uint8_t             num_axis;   /*number of axis*/
        uint8_t             cWheelId;   /*Color wheel index*/
        float               x_range[2]; /*[0]->minimal value from all lines in plot, [1] -> maximum value of all lines in plot.
                                        Or if x-axis fix_lines == TRUE it is equal to range[]*/
        float               y_range[2]; /*[0]->minimal value from all lines in plot, [1] -> maximum value of all lines in plot.
                                        Or if y-axis fix_lines == TRUE it is equal to range[]*/
        struct g_rec_cord   oDSpace;    /*relative dimension(in plot space) of data space(needed for points rendering->for cropping)*/
        struct g_rec_cord   tDSpace;    /*absolute dimension(in window space -because of glScissor) of data space(needed for points rendering->for cropping)*/
        struct g_axis       *axis;      /*plot axis*/
        struct g_line       *lines;     /*array of g_lines structures representin plot lines*/
        struct Color        bc_color;   /*plot background color*/
        struct g_rec_cord   oCord;      /*original coordinace->relative to figure(in range [0-1])*/
        struct g_rec_cord   tCord;      /*true cord(in range of actual size of window)->real coordinace, can be
                                        different compare to oCord->due to rescaling ...*/
};

struct g_figure{
        struct g_title      title;     /*Main Title*/
        unsigned int        num_plots;  /*number of plot which are stored in plots array*/
        struct g_plot       *plots;     /*array of plot*/
        struct g_title      *texts;     /*cover all others text fields in figure*/
        struct Color        color;      /*figure color*/
        Boolean             resize;     /*if True than also width and height will be resize according to window size change*/
        struct g_rec_cord   oCord;      /*original coordinace->relative to plot axis range(in range [0-1])*/
        struct g_rec_cord   tCord;      /*true cord(in range of actual size of window)->real coordinace, can be
                                        different compare to oCord->due to rescaling ...*/
};


extern struct g_figure create_figure(float x, float y, float w, float h);
/*create figure whose left down corner has coordinance x,y and whose width is w and height h.
All this coordinace has to be in range[0,1]*/
extern int add_plot(struct g_figure *fig, float x , float y, float w, float h);
/*create new plot in figure fig on position x,y with width w and height h, function return plot id(equal to
actaul plot index in plots array), if error ocur -1 is returned*/
extern int add_axis(struct g_plot *plt, float x1, float y1, float x2, float y2, G_AXIS_TYPE type);
/*add new axis in given plot, unlike the add_plot and create_figure, axis is define by wo points [x1,y1] and [x2,y2]
the values of this points has to be in range [0,1] ->relative position within plot area*/
int add_xyline(struct g_plot *plt, float *xdata, float *ydata, unsigned int n_points);
/*add new line in given plot, n__points specified number of x_data and y_data points*/
extern void drawFigure(struct g_figure *fig);
extern void rescaleFigure(struct g_figure *fig);
extern void figureDelete(struct g_figure *fig);
extern int setLineData(struct g_plot *plt, unsigned int line_id, float *x_data, float *y_data, unsigned int n_points);
extern int lineAddData(struct g_plot *plt, unsigned int line_id, float *x_data, float *y_data, unsigned int n_points);
extern void set_plotBcColor(struct g_plot *plt, struct Color c);
/*set plot background color*/
#endif
